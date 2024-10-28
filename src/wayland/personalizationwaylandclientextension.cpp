// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "personalizationwaylandclientextension.h"

#include <qwaylandclientextension.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QGuiApplication>
#include <private/qguiapplication_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qwaylandwindow_p.h>

DGUI_BEGIN_NAMESPACE

class PersonalizationManager_: public PersonalizationManager {};
Q_GLOBAL_STATIC(PersonalizationManager_, personalizationManager)

PersonalizationManager::PersonalizationManager()
    : QWaylandClientExtensionTemplate<PersonalizationManager>(1)
{
    if (QGuiApplication::platformName() == QLatin1String("wayland")) {
        QtWaylandClient::QWaylandIntegration *waylandIntegration = static_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
        if (!waylandIntegration) {
            qWarning() << "waylandIntegration is nullptr!!!";
            return;
        }
        m_waylandDisplay = waylandIntegration->display();
        if (!m_waylandDisplay) {
            qWarning() << "waylandDisplay is nullptr!!!";
            return;
        }
        addListener();
    }
}

PersonalizationManager *PersonalizationManager::instance()
{
    return personalizationManager;
}

PersonalizationWindowContext *PersonalizationManager::getWindowContext(QWindow *window)
{
    if (!window) {
        qWarning() << "window is nullptr!!!";
        return nullptr;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
#else
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
#endif
    if (!waylandWindow) {
        qWarning() << "waylandWindow is nullptr!!!";
        return nullptr;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto surface = waylandWindow->surface();
#else
    auto surface = waylandWindow->waylandSurface()->object();
#endif
    if (!surface) {
        qWarning() << "waylandSurface is nullptr!!!";
        return nullptr;
    }
    auto context = get_window_context(surface);
    if (m_windowContext.isNull()) {
        m_windowContext.reset(new PersonalizationWindowContext(context));
    }
    return m_windowContext.data();
}

void PersonalizationManager::addListener()
{
    if (!m_waylandDisplay) {
        qWarning() << "waylandDisplay is nullptr!, skip addListener";
        return;
    }
    m_waylandDisplay->addRegistryListener(&handleListenerGlobal, this);
}

void PersonalizationManager::removeListener()
{
    if (!m_waylandDisplay) {
        qWarning() << "waylandDisplay is nullptr!, skip removeListener";
        return;
    }
    m_waylandDisplay->removeListener(&handleListenerGlobal, this);
}

void PersonalizationManager::handleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version)
{
    if (interface == treeland_personalization_manager_v1_interface.name) {
        PersonalizationManager *integration = static_cast<PersonalizationManager *>(data);
        if (!integration) {
            qWarning() << "integration is nullptr!!!";
            return;
        }

        integration->init(registry, id, version);
    }
}

void PersonalizationManager::setEnableTitleBar(QWindow *window, bool enable)
{
    auto windowContext = this->getWindowContext(window);
    if (!windowContext) {
        qWarning() << "windowContext is nullptr!";
        return;
    }
    windowContext->setEnableTitleBar(enable);
}

PersonalizationWindowContext::PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationWindowContext>(1)
    , QtWayland::treeland_personalization_window_context_v1(context)
{
}

void PersonalizationWindowContext::setEnableTitleBar(bool enable)
{
    if (m_noTitlebar == enable) {
        return;
    }

    m_noTitlebar = enable;
    set_titlebar(m_noTitlebar ? enable_mode::enable_mode_enable : enable_mode::enable_mode_disable);
}

DGUI_END_NAMESPACE
