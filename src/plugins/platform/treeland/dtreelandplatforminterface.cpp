// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtreelandplatforminterface.h"
#include "dtreelandplatforminterface_p.h"

#include "dplatformtheme.h"
#include "private/dplatforminterface_p.h"

DGUI_BEGIN_NAMESPACE
DTreelandPlatformInterfacePrivate::DTreelandPlatformInterfacePrivate(DTreelandPlatformInterface *qq)
    :DCORE_NAMESPACE::DObjectPrivate(qq)
{
}

DTreelandPlatformInterface(DPlatformTheme *platformTheme)
    : DPlatformInterface(platformTheme)
    , Core::DObject(*new DTreelandPlatformInterfacePrivate(this))
{

}

DGUI_END_NAMESPACE
