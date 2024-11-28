// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMINTERFACE_P_H
#define DTREELANDPLATFORMINTERFACE_P_H

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
    QWindow *m_window = nullptr;
    QQueue<std::function<void()>> m_pendingTasks;
    PersonalizationManager *m_manager = nullptr;
    PersonalizationWindowContext *m_windowContext = nullptr;
    bool m_isNoTitlebar = true;
};

DGUI_END_NAMESPACE

#endif // DNATIVESETTINGS_P_H
