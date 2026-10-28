// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>

#include <QtWaylandClient/QWaylandClientExtension>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <wayland-client-core.h>
#include "qwayland-treeland-decoration-unstable-v1.h"
#include <qwaylandclientextension.h>

#include <private/qguiapplication_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qwaylandwindow_p.h>
#include <private/qwaylanddisplay_p.h>

class TreelandDecorationContext : public QWaylandClientExtensionTemplate<TreelandDecorationContext>,
                                  public QtWayland::treeland_decoration_context_v1
{
public:
    TreelandDecorationContext(struct ::treeland_decoration_context_v1 *context)
    : QWaylandClientExtensionTemplate<TreelandDecorationContext>(1)
    , QtWayland::treeland_decoration_context_v1(context)
    {
        set_titlebar_mode(titlebar_mode_hide);
    }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    return 0;
}
