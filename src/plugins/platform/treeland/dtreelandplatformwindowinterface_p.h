// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMWINDOWINTERFACE_P_H
#define DTREELANDPLATFORMWINDOWINTERFACE_P_H

#include "dplatformwindowinterface_p_p.h"
#include "dtreelandplatformwindowinterface.h"
#include "personalizationwaylandclientextension.h"

#include <QQueue>

DGUI_BEGIN_NAMESPACE

class DTreeLandPlatformWindowInterfacePrivate : public DPlatformWindowInterfacePrivate
{
public:
    D_DECLARE_PUBLIC(DTreeLandPlatformWindowInterface)
    DTreeLandPlatformWindowInterfacePrivate(DTreeLandPlatformWindowInterface *qq);

public:
    QQueue<std::function<void()>> m_pendingTasks;
    PersonalizationManager *m_manager = nullptr;
    PersonalizationWindowContext *m_windowContext = nullptr;
    bool m_isNoTitlebar = true;
    bool m_isWindowBlur = false;
    int m_radius;
};

DGUI_END_NAMESPACE

#endif // DTREELANDPLATFORMWINDOWINTERFACE_P_H
