// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXCBPLATFORMINTERFACE_P_H
#define DXCBPLATFORMINTERFACE_P_H

#include "dxcbplatforminterface.h"
#include "plugins/dplatforminterface_p.h"

DGUI_BEGIN_NAMESPACE

class DNativeSettings;
class DPlatformTheme;

class DXCBPlatformInterfacePrivate : public DPlatformInterfacePrivate
{
    D_DECLARE_PUBLIC(DXCBPlatformInterface)
public:
    DXCBPlatformInterfacePrivate(DXCBPlatformInterface *qq);

public:
    DPlatformTheme *parent = nullptr;
    bool fallbackProperty = true;
    DNativeSettings *m_nativeSettings;
    QHash<QString, QString> m_properties;
};

DGUI_END_NAMESPACE

#endif // DNATIVESETTINGS_P_H
