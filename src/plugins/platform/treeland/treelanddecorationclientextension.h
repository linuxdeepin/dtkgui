// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TREELANDDECORATIONCLIENTEXTENSION_H
#define TREELANDDECORATIONCLIENTEXTENSION_H

#include "qwayland-treeland-decoration-unstable-v1.h"

#include <dtkgui_global.h>
#include <private/qwaylanddisplay_p.h>

#include <QtWaylandClient/QWaylandClientExtension>
#include <QWaylandClientExtensionTemplate>

DGUI_BEGIN_NAMESPACE

class TreelandDecorationManager : public QWaylandClientExtensionTemplate<TreelandDecorationManager>,
                                  public QtWayland::treeland_decoration_manager_v1
{
    Q_OBJECT
public:
    static TreelandDecorationManager *instance();

    [[nodiscard]] inline bool isSupported() const { return m_isSupported; }

private:
    void addListener();
    void removeListener();

    explicit TreelandDecorationManager();
    static void handleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version);

private:
    QtWaylandClient::QWaylandDisplay *m_waylandDisplay = nullptr;
    bool m_isSupported;
};

class TreelandDecorationContext : public QWaylandClientExtensionTemplate<TreelandDecorationContext>,
                                 public QtWayland::treeland_decoration_context_v1
{
    Q_OBJECT
public:
    explicit TreelandDecorationContext(struct ::treeland_decoration_context_v1 *context);
    ~TreelandDecorationContext() override;
};

DGUI_END_NAMESPACE

#endif // TREELANDDECORATIONCLIENTEXTENSION_H
