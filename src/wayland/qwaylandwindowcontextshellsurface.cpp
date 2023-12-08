// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dcontextshellwindow.h"
#include "qwaylandwindowcontextshellsurface_p.h"

#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

Q_LOGGING_CATEGORY(layershellsurface, "dde.shell.layershell.surface")

QWaylandWindowContextSurface::QWaylandWindowContextSurface(
    QtWayland::treeland_personalization_manager_v1 *shell, QtWaylandClient::QWaylandWindow *window)
    : QtWaylandClient::QWaylandShellSurface(window)
    , QtWayland::treeland_window_context_v1()
    , m_dcontextShellWindow(DContextShellWindow::get(window->window()))
{
    init(shell->get_window_context(window->waylandSurface()->object()));
    set_no_titlebar(m_dcontextShellWindow->noTitlebar());
    connect(m_dcontextShellWindow, &DContextShellWindow::noTitlebarChanged, this, [this, window]() {
        set_no_titlebar(m_dcontextShellWindow->noTitlebar());
        window->waylandSurface()->commit();
    });
}

QWaylandWindowContextSurface::~QWaylandWindowContextSurface()
{
    destroy();
}
