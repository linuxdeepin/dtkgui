// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "treelanddecorationclientextension.h"

#include <private/qguiapplication_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qwaylandwindow_p.h>

#include <QGuiApplication>
#include <QDebug>

DGUI_BEGIN_NAMESPACE

TreelandDecorationManager::TreelandDecorationManager()
    : QWaylandClientExtensionTemplate<TreelandDecorationManager>(treeland_decoration_manager_v1_interface.version)
    , m_isSupported(false)
{
    QtWaylandClient::QWaylandIntegration *waylandIntegration = static_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
    if (!waylandIntegration) {
        qWarning() << "waylandIntegration is nullptr!!!";
        return;
    }
    m_waylandDisplay = waylandIntegration->display();
    if (m_waylandDisplay == nullptr) {
        qWarning() << "waylandDisplay is nullptr!!!";
        return;
    }
    addListener();
    m_isSupported = m_waylandDisplay->hasRegistryGlobal(QString::fromUtf8(treeland_decoration_manager_v1_interface.name));
    if (!m_isSupported) {
        qWarning() << "TreelandDecorationManager is not support";
    }
}

void TreelandDecorationManager::addListener()
{
    m_waylandDisplay->addRegistryListener(&handleListenerGlobal, this);
}

void TreelandDecorationManager::removeListener()
{
    m_waylandDisplay->removeListener(&handleListenerGlobal, this);
}

void TreelandDecorationManager::handleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version)
{
    if (interface == treeland_decoration_manager_v1_interface.name) {
        TreelandDecorationManager *integration = static_cast<TreelandDecorationManager *>(data);
        if (!integration) {
            qWarning() << "integration is nullptr!!!";
            return;
        }
        integration->init(registry, id, version);
    }
}

TreelandDecorationManager *TreelandDecorationManager::instance()
{
    static TreelandDecorationManager instance;
    return &instance;
}

TreelandDecorationContext::TreelandDecorationContext(struct ::treeland_decoration_context_v1 *context)
    : QWaylandClientExtensionTemplate<TreelandDecorationContext>(treeland_decoration_context_v1_interface.version)
    , QtWayland::treeland_decoration_context_v1(context)
{
}

TreelandDecorationContext::~TreelandDecorationContext()
{
    if (isInitialized())
        destroy();
}

DGUI_END_NAMESPACE
