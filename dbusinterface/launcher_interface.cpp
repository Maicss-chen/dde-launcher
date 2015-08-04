/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -N -p launcher_interface.h:coplauncher_interface.cpp -c LauncherInterface launcherinterface.xml
 *
 * qdbusxml2cpp is Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "launcher_interface.h"

/*
 * Implementation of interface class LauncherInterface
 */

LauncherInterface::LauncherInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    CategoryInfo::registerMetaType();
    qRegisterMetaType<CategoryInfoList>("CategoryInfoList");
    qDBusRegisterMetaType<CategoryInfoList>();

    AppFrequencyInfo::registerMetaType();
    qRegisterMetaType<AppFrequencyInfoList>("AppFrequencyInfoList");
    qDBusRegisterMetaType<AppFrequencyInfoList>();

    AppInstalledTimeInfo::registerMetaType();
    qRegisterMetaType<AppInstalledTimeInfoList>("AppInstalledTimeInfoList");
    qDBusRegisterMetaType<AppInstalledTimeInfoList>();

    ItemInfo::registerMetaType();
    qRegisterMetaType<AllItemInfosList>("AllItemInfosList");
    qDBusRegisterMetaType<AllItemInfosList>();
}

LauncherInterface::~LauncherInterface()
{
}
