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
    : QWaylandClientExtensionTemplate<PersonalizationManager>(treeland_personalization_manager_v1_interface.version)
    , m_isSupported(false)
{
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
    m_isSupported = m_waylandDisplay->hasRegistryGlobal(QString::fromUtf8(treeland_personalization_manager_v1_interface.name));
    if (!m_isSupported) {
        qWarning() << "PersonalizationManager is not support";
    }
}

PersonalizationManager::~PersonalizationManager()
{
    qDeleteAll(m_windowContexts);
    m_windowContexts.clear();
}

PersonalizationManager *PersonalizationManager::instance()
{
    return personalizationManager;
}

bool PersonalizationManager::isSupported() const
{
    return m_isSupported;
}

PersonalizationWindowContext *PersonalizationManager::getWindowContext(QWindow *window)
{
    if (!m_isSupported) {
        return nullptr;
    }
    if (!window) {
        qWarning() << "window is nullptr!!!";
        return nullptr;
    }
    if (m_windowContexts.contains(window)) {
        return m_windowContexts.value(window);
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

    // FIXME: Before calling get_window_context, it was not determined whether PersonalizationManager was isActive
    auto context =  new PersonalizationWindowContext(get_window_context(surface));
    connect(window, &QWindow::visibleChanged, context, [this, context, window](bool visible){
        if (!visible) {
            context->deleteLater();
            m_windowContexts.remove(window);
        }
    });

    m_windowContexts.insert(window, context);
    return context;
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
    auto windowContext = getWindowContext(window);
    if (!windowContext) {
        qWarning() << "windowContext is nullptr!";
        return;
    }
    windowContext->setEnableTitleBar(enable);
}

PersonalizationWindowContext::PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationWindowContext>(treeland_personalization_window_context_v1_interface.version)
    , QtWayland::treeland_personalization_window_context_v1(context)
{
}

void PersonalizationWindowContext::setEnableTitleBar(bool enable)
{
    set_titlebar(enable ? enable_mode::enable_mode_enable : enable_mode::enable_mode_disable);
}

DGUI_END_NAMESPACE
