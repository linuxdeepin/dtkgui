// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qwaylandpersonalizationshellintegration_p.h"
#include "qwaylandwindowcontextshellsurface_p.h"
#include "wayland-treeland-personalization-manager-v1-client-protocol.h"

QWaylandPersonalizationShellIntegration::QWaylandPersonalizationShellIntegration()
    : QWaylandShellIntegrationTemplate<QWaylandPersonalizationShellIntegration>(4)
{
}

QWaylandPersonalizationShellIntegration::~QWaylandPersonalizationShellIntegration()
{
    if (object()
        && treeland_personalization_manager_v1_get_version(object())
            >= TREELAND_WINDOW_CONTEXT_V1_DESTROY_SINCE_VERSION) {
        treeland_personalization_manager_v1_destroy(object());
    }
}

QtWaylandClient::QWaylandShellSurface *
QWaylandPersonalizationShellIntegration::createShellSurface(QtWaylandClient::QWaylandWindow *window)
{
    return new QWaylandWindowContextSurface(this, window);
}
