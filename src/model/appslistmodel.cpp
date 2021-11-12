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

#include "appslistmodel.h"
#include "appsmanager.h"
#include "src/global_util/calculate_util.h"
#include "src/global_util/constants.h"
#include "src/dbusinterface/dbusvariant/iteminfo.h"

#include <QSize>
#include <QDebug>
#include <QPixmap>
#include <QSettings>
#include <DHiDPIHelper>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static QMap<int, AppsListModel::AppCategory> CateGoryMap {
    { 0,  AppsListModel::Internet    },
    { 1,  AppsListModel::Chat        },
    { 2,  AppsListModel::Music       },
    { 3,  AppsListModel::Video       },
    { 4,  AppsListModel::Graphics    },
    { 5,  AppsListModel::Game        },
    { 6,  AppsListModel::Office      },
    { 7,  AppsListModel::Reading     },
    { 8,  AppsListModel::Development },
    { 9,  AppsListModel::System      },
    { 10, AppsListModel::Others      }
};

static QGSettings *gSetting = new QGSettings("com.deepin.dde.launcher", "/com/deepin/dde/launcher/");

const QStringList sysHideOpenPackages()
{
    QStringList hideOpen_list;
    //从gschema读取隐藏打开功能软件列表
    if (gSetting->keys().contains("appsHideOpenList")) {
        hideOpen_list << gSetting->get("apps-hide-open-list").toStringList();
    }

    return hideOpen_list;
}

const QStringList sysHideSendToDesktopPackages()
{
    QStringList hideSendToDesktop_list;
    //从gschema读取隐藏发送到桌面功能软件列表
    if (gSetting->keys().contains("appsHideSendToDesktopList")) {
        hideSendToDesktop_list << gSetting->get("apps-hide-send-to-desktop-list").toStringList();
    }

    return hideSendToDesktop_list;
}

const QStringList sysHideSendToDockPackages()
{
    QStringList hideSendToDock_list;
    //从gschema读取隐藏发送到Ｄock功能软件列表
    if (gSetting->keys().contains("appsHideSendToDockList")) {
        hideSendToDock_list << gSetting->get("apps-hide-send-to-dock-list").toStringList();
    }

    return hideSendToDock_list;
}

const QStringList sysHideStartUpPackages()
{
    QStringList hideStartUp_list;
    //从gschema读取隐藏开机启动功能软件列表
    if (gSetting->keys().contains("appsHideStartUpList")) {
        hideStartUp_list << gSetting->get("apps-hide-start-up-list").toStringList();
    }

    return hideStartUp_list;
}

const QStringList sysHideUninstallPackages()
{
    QStringList hideUninstall_list;
    //从gschema读取隐藏开机启动功能软件列表
    if (gSetting->keys().contains("appsHideUninstallList")) {
        hideUninstall_list << gSetting->get("apps-hide-uninstall-list").toStringList();
    }

    return hideUninstall_list;
}

const QStringList sysCantOpenPackages()
{
    QStringList cantOpen_list;
    //从gschema读取不可打开软件列表
    if (gSetting->keys().contains("appsCanNotOpenList")) {
        cantOpen_list << gSetting->get("apps-can-not-open-list").toStringList();
    }

    return cantOpen_list;
}

const QStringList sysCantSendToDesktopPackages()
{
    QStringList cantSendToDesktop_list;
    //从gschema读取不可发送到桌面软件列表
    if (gSetting->keys().contains("appsCanNotSendToDesktopList")) {
        cantSendToDesktop_list << gSetting->get("apps-can-not-send-to-desktop-list").toStringList();
    }

    return cantSendToDesktop_list;
}

const QStringList sysCantSendToDockPackages()
{
    QStringList cantSendToDock_list;
    //从gschema读取不可发送到Dock软件列表
    if (gSetting->keys().contains("appsCanNotSendToDockList")) {
        cantSendToDock_list << gSetting->get("apps-can-not-send-to-dock-list").toStringList();
    }

    return cantSendToDock_list;
}

const QStringList sysCantStartUpPackages()
{
    QStringList cantStartUp_list;
    //从gschema读取不可自动启动软件列表
    if (gSetting->keys().contains("appsCanNotStartUpList")) {
        cantStartUp_list << gSetting->get("apps-can-not-start-up-list").toStringList();
    }

    return cantStartUp_list;
}

const QStringList sysHoldPackages()
{
    //从先/etc/deepin-installer.conf读取不可卸载软件列表
    const QSettings settings("/etc/deepin-installer.conf", QSettings::IniFormat);
    auto holds_list = settings.value("dde_launcher_hold_packages").toStringList();

    //再从gschema读取不可卸载软件列表
    QGSettings setting("com.deepin.dde.launcher", "/com/deepin/dde/launcher/");
    if (setting.keys().contains("appsHoldList")) {
        holds_list << setting.get("apps-hold-list").toStringList();
    }

    //然后代码中补充固定的不可卸载软件列表
    holds_list << "dde-control-center"
               << "dde-computer"
               << "dde-trash"
               << "dde-file-manager"
               << "deepin-appstore"
               << "deepin-app-store"
               << "deepin-toggle-desktop"
               << "deepin-wm-multitaskingview"
               << "dde-calendar"
               << "deepin-manual"
               << "deepin-terminal"
               << "deepin-defender"
               << "kwin-wm-multitaskingview"
               << "dde-printer";

    return holds_list;
}

AppsListModel::AppsListModel(const AppCategory &category, QObject *parent)
    : QAbstractListModel(parent)
    , m_appsManager(AppsManager::instance())
    , m_calcUtil(CalculateUtil::instance())
    , m_hideOpenPackages(sysHideOpenPackages())
    , m_hideSendToDesktopPackages(sysHideSendToDesktopPackages())
    , m_hideSendToDockPackages(sysHideSendToDockPackages())
    , m_hideStartUpPackages(sysHideStartUpPackages())
    , m_hideUninstallPackages(sysHideUninstallPackages())
    , m_cantOpenPackages(sysCantOpenPackages())
    , m_cantSendToDesktopPackages(sysCantSendToDesktopPackages())
    , m_cantSendToDockPackages(sysCantSendToDockPackages())
    , m_cantStartUpPackages(sysCantStartUpPackages())
    , m_holdPackages(sysHoldPackages())
    , m_category(category)
    , m_drawBackground(true)
    , m_pageIndex(0)
{
    connect(m_appsManager, &AppsManager::dataChanged, this, &AppsListModel::dataChanged);
    connect(m_appsManager, &AppsManager::layoutChanged, this, &AppsListModel::layoutChanged);
    connect(m_appsManager, &AppsManager::itemDataChanged, this, &AppsListModel::itemDataChanged);
}

void AppsListModel::setCategory(const AppsListModel::AppCategory category)
{
    m_category = category;

    emit QAbstractListModel::layoutChanged();
}

///
/// \brief AppsListModel::setDraggingIndex mark current item as dragging item
/// \param index item index
///
void AppsListModel::setDraggingIndex(const QModelIndex &index)
{
    m_dragStartIndex = index;
    m_dragDropIndex = index;

    emit QAbstractListModel::dataChanged(index, index);
}

void AppsListModel::setDragDropIndex(const QModelIndex &index)
{
    if (m_dragDropIndex == index)
        return;
    //    if (m_dragDropIndex == m_dragStartIndex)
    //        return;

    m_dragDropIndex = index;

    emit QAbstractListModel::dataChanged(m_dragStartIndex, index);
}

///
/// \brief AppsListModel::dropInsert restore item from appsManager stash list
/// \param appKey item token in stash list
/// \param pos insert position, if pos is negetive, insert into front
///
void AppsListModel::dropInsert(const QString &appKey, const int pos)
{
    beginInsertRows(QModelIndex(), pos, pos);
    int appPos = m_pageIndex * m_calcUtil->appPageItemCount(m_category) + pos;
    m_appsManager->restoreItem(appKey, appPos);
    endInsertRows();
}

///
/// \brief AppsListModel::dropSwap drop m_draingIndex to nextPos
/// \param nextPos m_draggingIndex insert position
///
void AppsListModel::dropSwap(const int nextPos)
{
    if (!m_dragStartIndex.isValid())
        return;

    const QString appKey = m_dragStartIndex.data(AppsListModel::AppKeyRole).toString();

    removeRows(m_dragStartIndex.row(), 1, QModelIndex());
    dropInsert(appKey, nextPos);

    emit QAbstractItemModel::dataChanged(m_dragStartIndex, m_dragDropIndex);

    m_dragStartIndex = m_dragDropIndex = index(nextPos);
}

///
/// \brief AppsListModel::clearDraggingIndex reset dragging item record
///
void AppsListModel::clearDraggingIndex()
{
    const QModelIndex startIndex = m_dragStartIndex;
    const QModelIndex endIndex = m_dragDropIndex;

    m_dragStartIndex = m_dragDropIndex = QModelIndex();

    emit QAbstractItemModel::dataChanged(startIndex, endIndex);
}

int AppsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    int nSize = m_appsManager->appsInfoListSize(m_category);
    int pageCount = m_calcUtil->appPageItemCount(m_category);
    int nPageCount = nSize - pageCount * m_pageIndex;
    nPageCount = nPageCount > 0 ? nPageCount : 0;

    if(!m_calcUtil->fullscreen()){
        return nSize;
    }

    return qMin(pageCount, nPageCount);
}

const QModelIndex AppsListModel::indexAt(const QString &appKey) const
{
    int i = 0;
    const int count = rowCount(QModelIndex());
    while (i != count) {
        if (index(i).data(AppKeyRole).toString() == appKey)
            return index(i);
        ++i;
    }

    Q_UNREACHABLE();
}

void AppsListModel::setDrawBackground(bool draw)
{
    if (draw == m_drawBackground) return;

    m_drawBackground = draw;

    emit QAbstractItemModel::dataChanged(QModelIndex(), QModelIndex());
}

bool AppsListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)

    // TODO: not support remove multiple rows
    Q_ASSERT(count == 1);

    beginRemoveRows(parent, row, row);
    m_appsManager->stashItem(index(row));
    endRemoveRows();

    return true;
}

bool AppsListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    // disable invalid drop
    if (data->data("RequestDock").isEmpty())
        return false;

    // 全屏搜索模式不支持拖拽
    if (m_category == Search) {
        return  false;
    }

    return true;
}

QMimeData *AppsListModel::mimeData(const QModelIndexList &indexes) const
{
    // only allow drag 1 item
    Q_ASSERT(indexes.size() == 1);

    const QModelIndex index = indexes.first();

    QMimeData *mime = new QMimeData;
    mime->setData("RequestDock", index.data(AppDesktopRole).toByteArray());
    mime->setData("AppKey", index.data(AppKeyRole).toByteArray());

    if (index.data(AppIsRemovableRole).toBool())
        mime->setData("Removable", "");

    // this object will be delete in drag event finished.
    return mime;
}

QVariant AppsListModel::data(const QModelIndex &index, int role) const
{

    int nSize = m_appsManager->appsInfoListSize(m_category);
    int nFixCount = m_calcUtil->appPageItemCount(m_category);
    int pageCount = qMin(nFixCount, nSize - nFixCount * m_pageIndex);
    if(!m_calcUtil->fullscreen()) pageCount = nSize;
    if (!index.isValid() || index.row() >= pageCount)
        return QVariant();

   int start = nFixCount * m_pageIndex;
   const ItemInfo itemInfo = m_appsManager->appsInfoListIndex(m_category,start + index.row());

    switch (role) {
    case AppRawItemInfoRole:
        return QVariant::fromValue(itemInfo);
    case AppNameRole:
        return itemInfo.m_name;
    case AppDesktopRole:
        return itemInfo.m_desktop;
    case AppKeyRole:
        return itemInfo.m_key;
    case AppIconKeyRole:
        return itemInfo.m_iconKey;
    case AppCategoryRole:
        return QVariant::fromValue(itemInfo.category());
    case AppGroupRole:
        return QVariant::fromValue(m_category);
    case AppAutoStartRole:
        return m_category != Category ? m_appsManager->appIsAutoStart(itemInfo.m_desktop) : false;
    case AppIsOnDesktopRole:
        return m_appsManager->appIsOnDesktop(itemInfo.m_key);
    case AppIsOnDockRole:
        return m_appsManager->appIsOnDock(itemInfo.m_desktop);
    case AppIsRemovableRole:
        return !m_holdPackages.contains(itemInfo.m_key);
    case AppIsProxyRole:
        return m_appsManager->appIsProxy(itemInfo.m_key);
    case AppEnableScalingRole:
        return m_appsManager->appIsEnableScaling(itemInfo.m_key);
    case AppNewInstallRole: {
        if (m_category == Category) {
            const ItemInfoList &list = m_appsManager->appsInfoList(CateGoryMap[itemInfo.m_categoryId]);
            for (const ItemInfo &in : list) {
                if (m_appsManager->appIsNewInstall(in.m_key)) return true;
            }
        }

        return m_appsManager->appIsNewInstall(itemInfo.m_key);
    }
    case AppIconRole:
        return m_appsManager->appIcon(itemInfo, m_calcUtil->appIconSize().width());
    case AppDialogIconRole:
        return m_appsManager->appIcon(itemInfo, 36 * qApp->devicePixelRatio());
    case AppDragIconRole:
        return m_appsManager->appIcon(itemInfo, m_calcUtil->appIconSize().width() * 1.2);
    case ItemSizeHintRole:
        return m_calcUtil->appItemSize();
    case AppIconSizeRole:
        return m_calcUtil->appIconSize();
    case AppFontSizeRole:
        return DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6);
    case AppItemIsDraggingRole:
        return indexDragging(index);
    case CategoryEnterIconRole:
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            return DHiDPIHelper::loadNxPixmap(":/widgets/images/enter_details_normal-dark.svg");
        } else {
            return DHiDPIHelper::loadNxPixmap(":/widgets/images/enter_details_normal.svg");
        }
    case DrawBackgroundRole:
        return m_drawBackground;
    case AppHideOpenRole:
        return m_hideOpenPackages.contains(itemInfo.m_key);
    case AppHideSendToDesktopRole:
        return m_hideSendToDesktopPackages.contains(itemInfo.m_key);
    case AppHideSendToDockRole:
        return m_hideSendToDockPackages.contains(itemInfo.m_key);
    case AppHideStartUpRole:
        return m_hideStartUpPackages.contains(itemInfo.m_key);
    case AppHideUninstallRole:
        return m_hideUninstallPackages.contains(itemInfo.m_key);
    case AppCanOpenRole:
        return !m_cantOpenPackages.contains(itemInfo.m_key);
    case AppCanSendToDesktopRole:
        return !m_cantSendToDesktopPackages.contains(itemInfo.m_key);
    case AppCanSendToDockRole:
        return !m_cantSendToDockPackages.contains(itemInfo.m_key);
    case AppCanStartUpRole:
        return !m_cantStartUpPackages.contains(itemInfo.m_key);
    default:;
    }

    return QVariant();
}

Qt::ItemFlags AppsListModel::flags(const QModelIndex &index) const
{
    const Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if (m_category == All)
        return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    else
        return defaultFlags;
}

///
/// \brief AppsListModel::dataChanged tell view the appManager data is changed
/// \param category data category
///
void AppsListModel::dataChanged(const AppCategory category)
{
    if (category == All || category == m_category)
        emit QAbstractItemModel::layoutChanged();
    //        emit QAbstractItemModel::dataChanged(index(0), index(rowCount(QModelIndex())));
}

///
/// \brief AppsListModel::layoutChanged tell view the app layout is changed, such as appItem size, icon size, etc.
/// \param category data category
///
void AppsListModel::layoutChanged(const AppsListModel::AppCategory category)
{
    if (category == All || category == m_category)
        emit QAbstractItemModel::dataChanged(QModelIndex(), QModelIndex());
}

bool AppsListModel::indexDragging(const QModelIndex &index) const
{
    if (!m_dragStartIndex.isValid() || !m_dragDropIndex.isValid())
        return false;

    const int start = m_dragStartIndex.row();
    const int end = m_dragDropIndex.row();
    const int current = index.row();

    return (start <= end && current >= start && current <= end) ||
            (start >= end && current <= start && current >= end);
}

void AppsListModel::itemDataChanged(const ItemInfo &info)
{
    int i = 0;
    const int count = rowCount(QModelIndex());
    while (i != count) {
        if (index(i).data(AppKeyRole).toString() == info.m_key) {
            const QModelIndex modelIndex = index(i);
            emit QAbstractItemModel::dataChanged(modelIndex, modelIndex);
            return;
        }
        ++i;
    }
}

//bool AppsListModel::itemIsRemovable(const QString &desktop) const
//{
//    return m_holdPackages.contains(desktop);
//    static QStringList blacklist;
//    if (blacklist.isEmpty()) {
//        QFile file(UninstallFilterFile);
//        if (file.open(QFile::ReadOnly)) {
//            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
//            QJsonObject obj = doc.object();
//            QJsonArray arr = obj["blacklist"].toArray();
//            foreach (QJsonValue val, arr) {
//                blacklist << val.toString();
//            }
//            file.close();
//        }
//    }

//    foreach (QString val, blacklist) {
//        if (desktop.endsWith(val)) {
//            return false;
//        }
//    }

//    return true;
//}
