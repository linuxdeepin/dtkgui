// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMINTERFACE_H
#define DTREELANDPLATFORMINTERFACE_H

#include "private/dplatforminterface_p.h"

#include <DObject>

DGUI_BEGIN_NAMESPACE

class DTreelandPlatformInterfacePrivate;
class DTreelandPlatformInterface : public QObject, public DPlatformInterface, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DTreelandPlatformInterface)
public:
    explicit DTreelandPlatformInterface(DPlatformTheme *platformTheme);
};

DGUI_END_NAMESPACE
#endif
