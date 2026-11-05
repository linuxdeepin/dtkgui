// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "treelandbackgroundeffectclientextension.h"

#include <private/qguiapplication_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qwaylandwindow_p.h>

#include <QGuiApplication>
#include <QDebug>

DGUI_BEGIN_NAMESPACE

BackgroundEffectManager::BackgroundEffectManager()
    : QWaylandClientExtensionTemplate<BackgroundEffectManager>(ext_background_effect_manager_v1_interface.version)
    , m_isSupported(false)
    , m_hasBlurCapability(false)
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
    m_isSupported = m_waylandDisplay->hasRegistryGlobal(QString::fromUtf8(ext_background_effect_manager_v1_interface.name));
    if (!m_isSupported) {
        qWarning() << "BackgroundEffectManager is not support";
    }
}

void BackgroundEffectManager::addListener()
{
    m_waylandDisplay->addRegistryListener(&handleListenerGlobal, this);
}

void BackgroundEffectManager::removeListener()
{
    m_waylandDisplay->removeListener(&handleListenerGlobal, this);
}

void BackgroundEffectManager::handleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version)
{
    if (interface == ext_background_effect_manager_v1_interface.name) {
        BackgroundEffectManager *integration = static_cast<BackgroundEffectManager *>(data);
        if (!integration) {
            qWarning() << "integration is nullptr!!!";
            return;
        }
        integration->init(registry, id, version);
    }
}

BackgroundEffectManager *BackgroundEffectManager::instance()
{
    static BackgroundEffectManager instance;
    return &instance;
}

void BackgroundEffectManager::ext_background_effect_manager_v1_capabilities(uint32_t flags)
{
    bool hadBlur = m_hasBlurCapability;
    m_hasBlurCapability = (flags & static_cast<uint32_t>(capability_blur)) != 0;
    if (hadBlur != m_hasBlurCapability) {
        emit blurCapabilityChanged();
    }
}

BackgroundEffectSurface::BackgroundEffectSurface(struct ::ext_background_effect_surface_v1 *surface)
    : QWaylandClientExtensionTemplate<BackgroundEffectSurface>(ext_background_effect_surface_v1_interface.version)
    , QtWayland::ext_background_effect_surface_v1(surface)
{
}

BackgroundEffectSurface::~BackgroundEffectSurface()
{
    if (isInitialized())
        destroy();
}

void BackgroundEffectSurface::setBlurRegion(const QRegion &region, QtWaylandClient::QWaylandDisplay *display)
{
    auto wlRegion = display->createRegion(region);
    set_blur_region(wlRegion);
    wl_region_destroy(wlRegion);
}

DGUI_END_NAMESPACE
