// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dcontextshellwindow.h"

#include <private/qwaylandwindow_p.h>
#include <qwayland-treeland-personalization-manager-v1.h>

#include <QtWaylandClient/private/qwaylandshellsurface_p.h>

class QWaylandWindowContextSurface : public QtWaylandClient::QWaylandShellSurface,
                                     public QtWayland::treeland_window_context_v1
{
    Q_OBJECT
public:
    QWaylandWindowContextSurface(QtWayland::treeland_personalization_manager_v1 *shell,
                                 QtWaylandClient::QWaylandWindow *window);
    ~QWaylandWindowContextSurface() override;

private:
    DContextShellWindow *m_dcontextShellWindow;
};
