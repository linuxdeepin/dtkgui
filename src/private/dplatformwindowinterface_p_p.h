// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMINTERFACE_P_P_H
#define DPLATFORMINTERFACE_P_P_H

#include "dplatformwindowinterface_p.h"

#include <DObjectPrivate>
#include <DObject>

class QWindow;

DGUI_BEGIN_NAMESPACE

class DPlatformWindowInterfacePrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    D_DECLARE_PUBLIC(DPlatformWindowInterface)
public:
    DPlatformWindowInterfacePrivate(DPlatformWindowInterface *qq);

public:
    QWindow *m_window = nullptr;
    DPlatformHandle *m_platformHandle = nullptr;
};

DGUI_END_NAMESPACE

#endif // DPLATFORMINTERFACE_P_P_H
