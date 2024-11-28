// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXCBPLATFORMINTERFACE_P_H
#define DXCBPLATFORMINTERFACE_P_H

#include "dplatformwindowinterface_p_p.h"
#include "dxcbplatformwindowinterface.h"

DGUI_BEGIN_NAMESPACE

class DXCBPlatformWindowInterfacePrivate : public DPlatformWindowInterfacePrivate
{
public:
    D_DECLARE_PUBLIC(DXCBPlatformWindowInterface)
    DXCBPlatformWindowInterfacePrivate(DXCBPlatformWindowInterface *qq);
};

DGUI_END_NAMESPACE

#endif // DNATIVESETTINGS_P_H
