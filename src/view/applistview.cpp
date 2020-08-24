/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "applistview.h"
#include "src/global_util/constants.h"
#include "src/delegate/applistdelegate.h"
#include "src/model/appslistmodel.h"

#include <QStyleOptionViewItem>
#include <QPropertyAnimation>
#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>
#include <QPainter>
#include <QTimer>
#include <QLabel>
#include <QDebug>
#include <QDrag>
#include <QScroller>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

AppListView::AppListView(QWidget *parent)
    : QListView(parent)
    , m_dropThresholdTimer(new QTimer(this))
    , m_scrollAni(new QPropertyAnimation(verticalScrollBar(), "value"))
    , m_opacityEffect(new QGraphicsOpacityEffect(this))
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_updateEnableSelectionByMouseTimer(nullptr)
{
    viewport()->setAutoFillBackground(false);
    m_scrollAni->setEasingCurve(QEasingCurve::OutQuint);
    m_scrollAni->setDuration(800);

    horizontalScrollBar()->setEnabled(false);
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(NoFrame);
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(SingleSelection);
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);
    setFixedWidth(300);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    // support drag and drop.
    setDragDropMode(QAbstractItemView::DragDrop);
    setMovement(QListView::Free);
    setDragEnabled(true);

    // init opacity effect.
    m_opacityEffect->setOpacity(1);
    setGraphicsEffect(m_opacityEffect);

    // init drop threshold timer.
    m_dropThresholdTimer->setInterval(DLauncher::APP_DRAG_SWAP_THRESHOLD);
    m_dropThresholdTimer->setSingleShot(true);

#ifndef DISABLE_DRAG_ANIMATION
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppListView::prepareDropSwap, Qt::QueuedConnection);
#else
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppListView::dropSwap);
#endif

#if QT_VERSION < QT_VERSION_CHECK(5,9,0)
    touchTapDistance = 15;
#else
    touchTapDistance = QGuiApplicationPrivate::platformTheme()->themeHint(QPlatformTheme::TouchDoubleTapDistance).toInt();
#endif

    connect(m_scrollAni, &QPropertyAnimation::valueChanged, this, &AppListView::handleScrollValueChanged);
    connect(m_scrollAni, &QPropertyAnimation::finished, this, &AppListView::handleScrollFinished);
}

const QModelIndex AppListView::indexAt(const int index) const
{
    return model()->index(index, 0, QModelIndex());
}

void AppListView::wheelEvent(QWheelEvent *e)
{
    int offset = -e->delta();

    m_scrollAni->stop();
    m_scrollAni->setStartValue(verticalScrollBar()->value());
    m_scrollAni->setEndValue(verticalScrollBar()->value() + offset * m_speedTime);
    m_scrollAni->start();
}

void AppListView::mouseMoveEvent(QMouseEvent *e)
{
    if (e->source() == Qt::MouseEventSynthesizedByQt) {
        if (QScroller::hasScroller(this)) {
            return;
        }

        if (m_updateEnableSelectionByMouseTimer && m_updateEnableSelectionByMouseTimer->isActive()) {
            const QPoint difference_pos = e->pos() - m_lastTouchBeginPos;
            if (qAbs(difference_pos.x()) > touchTapDistance || qAbs(difference_pos.y()) > touchTapDistance) {
                QScroller::grabGesture(this);
                QScroller *scroller = QScroller::scroller(this);
                QScrollerProperties sp;
                sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
                scroller->setScrollerProperties(sp);

                scroller->handleInput(QScroller::InputPress, e->localPos(), e->timestamp());
                scroller->handleInput(QScroller::InputMove, e->localPos(), e->timestamp());

                connect(scroller, &QScroller::stateChanged, this, [=](QScroller::State newstate) {
                    if (newstate == QScroller::Inactive) {
                        QScroller::scroller(this)->deleteLater();
                    }
                });
            }
            return;
        }
    }

    setState(NoState);
    blockSignals(false);

    const QModelIndex &index = indexAt(e->pos());
    const QPoint pos = e->pos();

    if (index.isValid() && !m_enableDropInside)
        Q_EMIT entered(index);
    else
        Q_EMIT entered(QModelIndex());

    if (e->button() != Qt::LeftButton)
        return;

    if (qAbs(pos.x() - m_dragStartPos.x()) > DLauncher::DRAG_THRESHOLD ||
        qAbs(pos.y() - m_dragStartPos.y()) > DLauncher::DRAG_THRESHOLD) {
        m_dragStartPos = e->pos();
        m_dragStartRow = index.row();
        return startDrag(index);
    }

    QListView::mouseMoveEvent(e);
}

void AppListView::mousePressEvent(QMouseEvent *e)
{
    if (e->source() == Qt::MouseEventSynthesizedByQt) {
        m_lastTouchBeginPos = e->pos();

        if (m_updateEnableSelectionByMouseTimer) {
            m_updateEnableSelectionByMouseTimer->stop();
        }
        else {
            m_updateEnableSelectionByMouseTimer = new QTimer(this);
            m_updateEnableSelectionByMouseTimer->setSingleShot(true);
            m_updateEnableSelectionByMouseTimer->setInterval(500);

            connect(m_updateEnableSelectionByMouseTimer, &QTimer::timeout, this, [=] {
                m_updateEnableSelectionByMouseTimer->deleteLater();
                m_updateEnableSelectionByMouseTimer = nullptr;
            });
        }
        m_updateEnableSelectionByMouseTimer->start();
        return;
    }

    const QModelIndex &index = indexAt(e->pos());
    if (!index.isValid())
        e->ignore();

    const bool isCategoryList = qobject_cast<AppsListModel*>(model())->category() == AppsListModel::Category;

    if (e->button() == Qt::RightButton && !isCategoryList) {
        const QPoint rightClickPoint = mapToGlobal(e->pos());
        const QModelIndex &clickedIndex = QListView::indexAt(e->pos());

        if (clickedIndex.isValid())
            emit popupMenuRequested(rightClickPoint, clickedIndex);
    }

    if (e->button() == Qt::LeftButton) {
        if (isCategoryList) {
            return;
        } else {
            m_dragStartPos = e->pos();
            m_dragStartRow = indexAt(e->pos()).row();
        }
    }

    QListView::mousePressEvent(e);
}

void AppListView::mouseReleaseEvent(QMouseEvent *e)
{
    if (QScroller::hasScroller(this)) {
        QScroller::scroller(this)->deleteLater();
        return;
    }

    const QModelIndex &index = indexAt(e->pos());
    if (!index.isValid()) {
        e->ignore();
        return;
    }

    if (qobject_cast<AppsListModel*>(model())->category() == AppsListModel::Category && e->button() == Qt::LeftButton) {
        emit requestSwitchToCategory(index);
        return;
    }

    if (e->source() == Qt::MouseEventSynthesizedByQt) {
        // reissue event
        emit clicked(index);
    }

    QListView::mouseReleaseEvent(e);
}

void AppListView::dragEnterEvent(QDragEnterEvent *e)
{
    const QModelIndex index = indexAt(e->pos());

    if (model()->canDropMimeData(e->mimeData(), e->dropAction(), index.row(), index.column(), QModelIndex())) {
        // to enable transparent.
        m_opacityEffect->setOpacity(0.5);

        return e->accept();
    }
}

void AppListView::dragMoveEvent(QDragMoveEvent *e)
{
    if (m_lastFakeAni)
        return;

    const QModelIndex dropIndex = QListView::indexAt(e->pos());
    if (dropIndex.isValid())
        m_dropToRow = dropIndex.row();

    m_dropThresholdTimer->stop();

    const QPoint pos = e->pos();
    const QRect rect = this->rect();

    if (pos.y() < DRAG_SCROLL_THRESHOLD) {
        Q_EMIT requestScrollUp();
    } else if (pos.y() > rect.height() - DRAG_SCROLL_THRESHOLD) {
        Q_EMIT requestScrollDown();
    } else {
        Q_EMIT requestScrollStop();

        // rolling without swapping.
        m_dropThresholdTimer->start();
    }

    // drag move does not allow to have selected effect.
    Q_EMIT entered(QModelIndex());
}

void AppListView::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();

    // drag leave will also restore opacity.
    m_opacityEffect->setOpacity(1);
    m_dropThresholdTimer->stop();

    Q_EMIT requestScrollStop();
}

void AppListView::dropEvent(QDropEvent *e)
{
    e->accept();

    // restore opacity.
    m_opacityEffect->setOpacity(1);
    m_enableDropInside = true;
}

void AppListView::enterEvent(QEvent *event)
{
    QListView::leaveEvent(event);

    emit requestEnter(true);
}

void AppListView::leaveEvent(QEvent *event)
{
    QListView::leaveEvent(event);

    emit requestEnter(false);
}

void AppListView::startDrag(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel || listModel->category() != AppsListModel::All)
        return;

    const QModelIndex &dragIndex = index;
    const QRect index_rect = visualRect(index);
    const auto ratio = devicePixelRatioF();
    const QSize rectSize = index_rect.size();

    // QPoint(50, 42) is shadow radius
    const QPoint hotSpot = m_dragStartPos - index_rect.topLeft() + QPoint(50, 42)  / ratio;

    QPixmap dropPixmap;
    QImage sourcePixmap(rectSize * ratio, QImage::Format_ARGB32_Premultiplied);
    sourcePixmap.fill(Qt::transparent);
    sourcePixmap.setDevicePixelRatio(ratio);

    QPainter painter(&sourcePixmap);
    QStyleOptionViewItem item;
    item.rect = QRect(QPoint(0, 0), rectSize);
    item.features |= QStyleOptionViewItem::Alternate;

    itemDelegate()->paint(&painter, item, index);
    dropPixmap = QPixmap::fromImage(sourcePixmap);

    // wm support transparent to draw a shadow effect.
    if (m_wmHelper->hasComposite()) {
        QColor shadowColor("#2CA7F8");
        shadowColor.setAlpha(180);
        dropPixmap = AppListDelegate::dropShadow(dropPixmap, 50, shadowColor, QPoint(0, 8));
        dropPixmap.setDevicePixelRatio(ratio);
    }

    QDrag *drag = new QDrag(this);
    drag->setMimeData(model()->mimeData(QModelIndexList() << dragIndex));
    drag->setPixmap(dropPixmap);
    drag->setHotSpot(hotSpot);

    // request remove current item.
    if (listModel->category() == AppsListModel::All) {
        m_dropToRow = index.row();
        listModel->setDraggingIndex(index);
    }

    setState(DraggingState);
    drag->exec(Qt::MoveAction);

    // disable animation when finally dropped
    m_dropThresholdTimer->stop();

    // disable auto scroll
    Q_EMIT requestScrollStop();

    if (listModel->category() != AppsListModel::All)
        return;

    if (!m_lastFakeAni) {
        if (m_enableDropInside)
            listModel->dropSwap(m_dropToRow);
        else
            listModel->dropSwap(m_dragStartRow);

        listModel->clearDraggingIndex();
    } else {
        connect(m_lastFakeAni, &QPropertyAnimation::finished, listModel, &AppsListModel::clearDraggingIndex);
    }

    m_enableDropInside = false;
}

void AppListView::handleScrollValueChanged()
{
    QScrollBar *vscroll = verticalScrollBar();

    if (vscroll->value() == vscroll->maximum() ||
        vscroll->value() == vscroll->minimum()) {
        blockSignals(false);
    } else {
        blockSignals(true);
    }
}

void AppListView::handleScrollFinished()
{
    blockSignals(false);

    QPoint pos = mapFromGlobal(QCursor::pos());
    emit entered(indexAt(pos));
}

void AppListView::prepareDropSwap()
{
    if (m_lastFakeAni || m_dropThresholdTimer->isActive())
        return;

    const QModelIndex dropIndex = indexAt(m_dropToRow);
    if (!dropIndex.isValid())
        return;

    const QModelIndex dragStartIndex = indexAt(m_dragStartRow);
    if (dropIndex == dragStartIndex)
        return;

    if (!dragStartIndex.isValid()) {
        m_dragStartRow = dropIndex.row();
        return;
    }

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->clearDraggingIndex();
    listModel->setDraggingIndex(dragStartIndex);
    listModel->setDragDropIndex(dropIndex);

    const int startIndex = dragStartIndex.row();
    const bool moveToNext = startIndex <= m_dropToRow;
    const int start = moveToNext ? startIndex : m_dropToRow;
    const int end = !moveToNext ? startIndex : m_dropToRow;

    if (start == end)
        return;

    for (int i = start + moveToNext; i != end - !moveToNext; ++i) {
        createFakeAnimation(i, moveToNext);
    }

    // last animation.
    createFakeAnimation(end - !moveToNext, moveToNext, true);

    m_dragStartRow = dropIndex.row();
}

void AppListView::createFakeAnimation(const int pos, const bool moveNext, const bool isLastAni)
{
    const QModelIndex index(indexAt(pos));

    QLabel *floatLabel = new QLabel(this);
    QPropertyAnimation *animation = new QPropertyAnimation(floatLabel, "pos", floatLabel);

    const auto ratio = devicePixelRatioF();
    const QSize rectSize(300, 36);

    QPixmap pixmap(rectSize * ratio);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(ratio);

    QStyleOptionViewItem item;
    item.rect = QRect(QPoint(0, 0), rectSize);
    item.features |= QStyleOptionViewItem::HasDisplay;

    QPainter painter(&pixmap);
    itemDelegate()->paint(&painter, item, index);

    floatLabel->setFixedSize(rectSize);
    floatLabel->setPixmap(pixmap);
    floatLabel->show();

    animation->setStartValue(visualRect(index).topLeft());
    animation->setEndValue(visualRect(indexAt(moveNext ? pos - 1 : pos + 1)).topLeft());
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setDuration(200);

    connect(animation, &QPropertyAnimation::finished, floatLabel, &QLabel::deleteLater);

    if (isLastAni) {
        m_lastFakeAni = animation;
        connect(animation, &QPropertyAnimation::finished, this, &AppListView::dropSwap, Qt::QueuedConnection);
        connect(animation, &QPropertyAnimation::valueChanged, m_dropThresholdTimer, &QTimer::stop);
    }

    animation->start();
}

void AppListView::dropSwap()
{
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());

    if (!listModel)
        return;

    listModel->dropSwap(m_dropToRow);

    m_lastFakeAni = nullptr;
    m_dragStartRow = m_dropToRow;

    setState(NoState);
}
