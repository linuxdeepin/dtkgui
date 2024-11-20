// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMINTERFACE_P_H
#define DTREELANDPLATFORMINTERFACE_P_H

#include "dtreelandplatforminterface.h"
#include "private/dplatforminterface_p.h"

#include <DObjectPrivate>

DGUI_BEGIN_NAMESPACE

class DTreelandPlatformInterfacePrivate : public DCORE_NAMESPACE::DObjectPrivate
{
public:
    D_DECLARE_PUBLIC(DTreelandPlatformInterface)
    DTreelandPlatformInterfacePrivate(DTreelandPlatformInterface *qq);

private:
};

DGUI_END_NAMESPACE

#endif // DNATIVESETTINGS_P_H
