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

#include "searchwidget.h"
#include "src/global_util/util.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QKeyEvent>
#include <dimagebutton.h>
#include <DDBusSender>
#include <DFloatingButton>
#include <DSearchEdit>

#define ICON_SIZE   24
#define BTN_SIZE    40

#define SEARCHEIT_WIDTH 350
#define SEARCHEIT_HEIGHT 40

DWIDGET_USE_NAMESPACE

SearchWidget::SearchWidget(QWidget *parent) :
    QFrame(parent)
{
    m_leftSpacing = new QFrame(this);
    m_rightSpacing = new QFrame(this);

    m_leftSpacing->setFixedWidth(0);
    m_rightSpacing->setFixedWidth(0);

    m_toggleCategoryBtn = new DFloatingButton(this);
    m_toggleCategoryBtn->setAccessibleName("mode-toggle-button");
    m_toggleCategoryBtn->setIcon(QIcon(":/icons/skin/icons/category_normal_22px.png"));
    m_toggleCategoryBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_toggleCategoryBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    m_toggleCategoryBtn->setAutoExclusive(true);
    m_toggleCategoryBtn->setBackgroundRole(DPalette::Button);

    m_toggleModeBtn = new DFloatingButton(this);
    m_toggleModeBtn->setIcon(QIcon(":/icons/skin/icons/unfullscreen_normal.png"));
    m_toggleModeBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_toggleModeBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    m_toggleModeBtn->setAutoExclusive(true);
    m_toggleModeBtn->setBackgroundRole(DPalette::Button);

    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->setAccessibleName("search-edit");
    m_searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_searchEdit->lineEdit()->setFixedSize(SEARCHEIT_WIDTH, SEARCHEIT_HEIGHT);

    DStyle::setFocusRectVisible(m_searchEdit->lineEdit(), false);
    setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_leftSpacing);
    mainLayout->addWidget(m_toggleCategoryBtn);
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchEdit);
    mainLayout->addStretch();
    mainLayout->addWidget(m_toggleModeBtn);
    mainLayout->addWidget(m_rightSpacing);
    mainLayout->addSpacing(30);

    setLayout(mainLayout);

    connect(m_searchEdit, &DSearchEdit::textChanged, [this] {
        auto searchStr = m_searchEdit->text();
        emit searchTextChanged(searchStr.mid(0, 1) + searchStr.mid(1).replace(" ", ""));
    });
    connect(m_toggleModeBtn, &DFloatingButton::clicked, this, [=] {
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
        DDBusSender()
        .service("com.deepin.dde.daemon.Launcher")
        .interface("com.deepin.dde.daemon.Launcher")
        .path("/com/deepin/dde/daemon/Launcher")
        .property("Fullscreen")
        .set(false);
#else
        const QStringList args{
            "--print-reply",
            "--dest=com.deepin.dde.daemon.Launcher",
            "/com/deepin/dde/daemon/Launcher",
            "org.freedesktop.DBus.Properties.Set",
            "string:com.deepin.dde.daemon.Launcher",
            "string:Fullscreen",
            "variant:boolean:false"
        };

        QProcess::startDetached("dbus-send", args);
#endif
    });
    connect(m_toggleCategoryBtn, &DFloatingButton::clicked, this, &SearchWidget::toggleMode);
}

DSearchEdit *SearchWidget::edit()
{
    return m_searchEdit;
}

DFloatingButton *SearchWidget::categoryBtn()
{
    return m_toggleCategoryBtn;
}

void SearchWidget::clearSearchContent()
{
    m_searchEdit->clearEdit();
    m_searchEdit->clear();
}

void SearchWidget::setLeftSpacing(int spacing)
{
    m_leftSpacing->setFixedWidth(spacing);
}

void SearchWidget::setRightSpacing(int spacing)
{
    m_rightSpacing->setFixedWidth(spacing);
}

void SearchWidget::showToggle()
{
    m_toggleCategoryBtn->show();
    m_toggleModeBtn->show();
}

void SearchWidget::hideToggle()
{
    m_toggleCategoryBtn->hide();
    m_toggleModeBtn->hide();
}

void SearchWidget::updateSize(double scaleX, double scaleY)
{
    m_searchEdit->lineEdit()->setFixedSize(SEARCHEIT_WIDTH * scaleX, SEARCHEIT_HEIGHT * scaleY);
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;
    m_toggleCategoryBtn->setFixedSize(BTN_SIZE * scale, BTN_SIZE * scale);
    m_toggleCategoryBtn->setIconSize(QSize(ICON_SIZE * scale, ICON_SIZE * scale));
    m_toggleModeBtn->setIconSize(QSize(ICON_SIZE * scale, ICON_SIZE * scale));
    m_toggleModeBtn->setFixedSize(BTN_SIZE * scale, BTN_SIZE * scale);
}

