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

#include "calculate_util.h"
#include "src/dbusinterface/monitorinterface.h"
#include "src/dbusinterface/dbusdisplay.h"

#include <QDebug>
#include <QScreen>
#include <QDesktopWidget>
#include <QApplication>

static const QString DisplayModeKey = "display-mode";
static const QString DisplayModeFree = "free";
static const QString DisplayModeCategory = "category";

QPointer<CalculateUtil> CalculateUtil::INSTANCE = nullptr;

CalculateUtil *CalculateUtil::instance()
{
    if (INSTANCE.isNull())
        INSTANCE = new CalculateUtil(nullptr);

    return INSTANCE;
}

void CalculateUtil::setDisplayMode(const int mode)
{
    m_launcherGsettings->set(DisplayModeKey, mode == ALL_APPS ? DisplayModeFree : DisplayModeCategory);
}

QSize CalculateUtil::appIconSize() const
{
    if (!m_launcherInter->fullscreen())
        return QSize(24, 24) * qApp->devicePixelRatio();

    QSize s(m_appItemSize, m_appItemSize);
    const double ratio = m_launcherGsettings->get("apps-icon-ratio").toDouble();
    return s * ratio;
}

double CalculateUtil::getScreenScaleX()
{
    int width = qApp->primaryScreen()->geometry().size().width();    
    return double(width) / 1920;
}

double CalculateUtil::getScreenScaleY()
{
    int width = qApp->primaryScreen()->geometry().size().height();
    return double(width) / 1080;
}

QSize CalculateUtil::getScreenSize() const
{
    int width = qApp->primaryScreen()->geometry().size().width();
    int height = qApp->primaryScreen()->geometry().size().height();
    return  QSize(width, height);
}

QSize CalculateUtil::getAppBoxSize()
{
    int width = qApp->primaryScreen()->geometry().size().width() * 0.51;
    int height = qApp->primaryScreen()->geometry().size().height() * 0.69;
    return  QSize(width, height);
}

bool CalculateUtil::increaseIconSize()
{
    const double value = m_launcherGsettings->get("apps-icon-ratio").toDouble();
    const double ratio = std::min(0.6, value + 0.1);

    if (qFuzzyCompare(value, ratio)) return false;

    m_launcherGsettings->set("apps-icon-ratio", ratio);
    return true;
}

bool CalculateUtil::decreaseIconSize()
{
    const double value = m_launcherGsettings->get("apps-icon-ratio").toDouble();
    const double ratio = std::max(0.2, value - 0.1);

    if (qFuzzyCompare(value, ratio)) return false;

    m_launcherGsettings->set("apps-icon-ratio", ratio);
    return true;
}

int CalculateUtil::displayMode() const
{
    const QString displayMode = m_launcherGsettings->get(DisplayModeKey).toString();

    if (displayMode == DisplayModeCategory) {
        return GROUP_BY_CATEGORY;
    }

    return ALL_APPS;
}

void CalculateUtil::calculateAppLayout(const QSize &containerSize, const int dockPosition)
{
    Q_UNUSED(dockPosition);

    const QRect pr = qApp->primaryScreen()->geometry();
    const int screenWidth = pr.width();
    const int spacing = pr.width() <= 1440 ? 10 : 28;
    // mini mode
    if (m_launcherGsettings->get(DisplayModeKey).toString() == DisplayModeCategory) {

        m_appMarginTop = 0;
        m_appColumnCount = 4;
        int Catespacing = 54;
        int calc_categroyitem_width = 0;
        int calc_categoryspacing = 0;
        if (pr.width() <= 1440) {
            Catespacing = 20;
            calc_categroyitem_width = (getAppBoxSize().width() - Catespacing * m_appColumnCount * 2) / m_appColumnCount + 0.5;
            calc_categoryspacing  = (double(getAppBoxSize().width()) - calc_categroyitem_width * m_appColumnCount) / (m_appColumnCount * 2) - 0.5;
            int calc_categoryspacing_height  = (double(containerSize.height()) - calc_categroyitem_width * 3  - 60) / (3 * 2) - 0.5;
            m_appMarginTop = (containerSize.height() - calc_categoryspacing_height * 6 - calc_categroyitem_width * 3) / 2;
            if (calc_categoryspacing > calc_categoryspacing_height)  calc_categoryspacing = calc_categoryspacing_height;
        } else {
            calc_categroyitem_width = (getAppBoxSize().width() - Catespacing * m_appColumnCount * 2) / m_appColumnCount + 0.5;
            calc_categoryspacing = (double(getAppBoxSize().width()) - calc_categroyitem_width * m_appColumnCount - Catespacing * 2) / (m_appColumnCount * 2) - 8;
        }

        m_appItemSpacing = calc_categoryspacing;
        m_appItemSize = calc_categroyitem_width;
        m_appItemFontSize = m_appItemSize <= 130 ? 8 : qApp->font().pointSize();
        m_appMarginLeft = (getAppBoxSize().width() - calc_categroyitem_width * m_appColumnCount - calc_categoryspacing * m_appColumnCount * 2) / 2;
        emit layoutChanged();
        return;
    }

    const int columns = 7;
    const int rows = m_appPageItemCount / columns;

    const int calc_item_width = int((double(containerSize.width()) - spacing * columns * 2) / columns + 0.5);
    const int calc_spacing = int((double(containerSize.width()) - calc_item_width * columns) / (columns * 2) - 0.5);

    double scaleX = getScreenScaleX();
    double scaleY = getScreenScaleY();
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;
    const int h = containerSize.height() - 15 - scale * 12; //35为 图标：12 下边距：15
    const int nRowSpace = int(double(h - calc_item_width * rows) / (rows * 2) - 0.5);
    int nSpace = qMin(calc_spacing, nRowSpace);
    m_appMarginLeft = (containerSize.width() - calc_item_width * columns - nSpace * columns * 2) / 2;
    m_appMarginTop = (containerSize.height() - calc_item_width * rows - nSpace * rows * 2) / 2 + nSpace;

    calculateTextSize(screenWidth);

    m_appItemSpacing = nSpace;
    m_appItemSize = calc_item_width;
    m_appColumnCount = columns;

    // calculate font size;
    m_appItemFontSize = m_appItemSize <= 80 ? 8 : qApp->font().pointSize();
    emit layoutChanged();
}

CalculateUtil::CalculateUtil(QObject *parent)
    : QObject(parent),
      m_launcherGsettings(new QGSettings("com.deepin.dde.launcher",
                                         "/com/deepin/dde/launcher/", this))
{
    m_launcherInter = new DBusLauncher(this);
}

void CalculateUtil::calculateTextSize(const int screenWidth)
{
    if (screenWidth > 1366) {
        m_navgationTextSize = 14;
        m_titleTextSize = 40;
    } else {
        m_navgationTextSize = 11;
        m_titleTextSize = 38;
    }
}
