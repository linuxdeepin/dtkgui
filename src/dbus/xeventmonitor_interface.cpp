// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "xeventmonitor_interface.h"

static void registerAreaListMetaType()
{
    qRegisterMetaType<AreaList>("AreaList");
    qDBusRegisterMetaType<AreaList>();
}
Q_CONSTRUCTOR_FUNCTION(registerAreaListMetaType);

ComDeepinApiXEventMonitorInterface::ComDeepinApiXEventMonitorInterface(const QString &service, const QString &path,
                                                                       const char *interface, QObject *parent,
                                                                       const QDBusConnection &con)
    : QDBusAbstractInterface(service, path, interface, con, parent)
{

}

ComDeepinApiXEventMonitorInterface::~ComDeepinApiXEventMonitorInterface()
{
}
#include "xeventmonitor_interface.moc"
