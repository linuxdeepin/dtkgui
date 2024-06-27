// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <private/qwaylandshellintegration_p.h>
#include <qwayland-treeland-personalization-manager-v1.h>

class QWaylandPersonalizationShellIntegration
    : public QtWaylandClient::QWaylandShellIntegrationTemplate<
          QWaylandPersonalizationShellIntegration>,
      public QtWayland::treeland_personalization_manager_v1
{
public:
    QWaylandPersonalizationShellIntegration();
    ~QWaylandPersonalizationShellIntegration() override;

    QtWaylandClient::QWaylandShellSurface *
    createShellSurface(QtWaylandClient::QWaylandWindow *window) override;
};
