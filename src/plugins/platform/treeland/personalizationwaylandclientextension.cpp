// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "personalizationwaylandclientextension.h"

#include <qglobal.h>
#include <qwaylandclientextension.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QGuiApplication>
#include <private/qguiapplication_p.h>
#include <private/qwaylandintegration_p.h>

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
        m_waylandDisplay.reset(waylandIntegration->display());
        if (m_waylandDisplay.isNull()) {
            qWarning() << "waylandDisplay is nullptr!!!";
            return;
        }
        addListener();
    }

    auto appearanceContext = get_appearance_context();
    if (!appearanceContext) {
        qWarning() << "appearanceContext is nullptr!!!";
        return;
    }
    m_appearanceContext.reset(new PersonalizationAppearanceContext(appearanceContext));
}

PersonalizationManager *PersonalizationManager::instance()
{
    return personalizationManager;
}

int PersonalizationManager::windowRadius() const
{
    if (m_appearanceContext.isNull()) {
        return 0;
    }
    return m_appearanceContext->windowRadius();
}

void PersonalizationManager::setWindowRadius(int radius)
{
    if (m_appearanceContext.isNull()) {
        return;
    }
    m_appearanceContext->setWindowRadius(radius);
}

QString PersonalizationManager::fontName() const
{
    if (m_appearanceContext.isNull()) {
        return QString();
    }
    return m_appearanceContext->fontName();
}

void PersonalizationManager::setFontName(const QString& fontName)
{
    if (m_appearanceContext.isNull()) {
        return;
    }
    m_appearanceContext->setFontName(fontName);
}

QString PersonalizationManager::monoFontName() const
{
    if (m_appearanceContext.isNull()) {
        return QString();
    }
    return m_appearanceContext->monoFontName();
}

void PersonalizationManager::setMonoFontName(const QString& monoFontName)
{
    if (m_appearanceContext.isNull()) {
        return;
    }
    m_appearanceContext->setMonoFontName(monoFontName);
}

QString PersonalizationManager::cursorThemeName() const
{
    if (m_appearanceContext.isNull()) {
        return QString();
    }
    return m_appearanceContext->cursorTheme();
}

void PersonalizationManager::setCursorThemeName(const QString& cursorTheme)
{
    if (m_appearanceContext.isNull()) {
        return;
    }
    m_appearanceContext->setCursorTheme(cursorTheme);
}

QString PersonalizationManager::iconThemeName() const
{
    if (m_appearanceContext.isNull()) {
        return QString();
    }
    return m_appearanceContext->iconTheme();
}

void PersonalizationManager::setIconThemeName(const QString& iconTheme)
{
    if (m_appearanceContext.isNull()) {
        return;
    }
    m_appearanceContext->setIconTheme(iconTheme);
}

PersonalizationWindowContext *PersonalizationManager::getWindowContext(QWindow *window)
{
    if (!window) {
        qWarning() << "window is nullptr!!!";
        return nullptr;
    }
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (!waylandWindow) {
        qWarning() << "waylandWindow is nullptr!!!";
        return nullptr;
    }
    auto surface = waylandWindow->waylandSurface()->object();
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
    m_waylandDisplay->addRegistryListener(&handleListenerGlobal, this);
}

void PersonalizationManager::removeListener()
{
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

bool PersonalizationManager::isEnabledNoTitlebar(QWindow *window)
{
    return m_isNoTitlebarMap.value(window);
}

bool PersonalizationManager::setEnabledNoTitlebar(QWindow *window, bool enable)
{
    auto windowContext = this->getWindowContext(window);
    if (!windowContext) {
        qWarning() << "windowContext is nullptr!";
        return false;
    }
    windowContext->setNoTitlebar(enable);
    m_isNoTitlebarMap.insert(window, enable);
    return true;
}

void PersonalizationManager::setWindowRadius(QWindow *window, int radius)
{
    auto windowContext = this->getWindowContext(window);
    if (!windowContext) {
        return;
    }
    windowContext->setWindowRadius(radius);
}

PersonalizationWindowContext::PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationWindowContext>(1)
    , QtWayland::treeland_personalization_window_context_v1(context)
{
}

void PersonalizationWindowContext::setNoTitlebar(bool enable)
{
    if (m_noTitlebar == enable) {
        return;
    }

    m_noTitlebar = enable;
    set_no_titlebar(m_noTitlebar ? enable_mode::enable_mode_enable : enable_mode::enable_mode_disable);
}

int PersonalizationWindowContext::windowRadius()
{
    return m_windowRadius;
}

void PersonalizationWindowContext::setWindowRadius(int radius)
{
    if (m_windowRadius == radius) {
        return;
    }

    m_windowRadius = radius;
    set_round_corner_radius(radius);
}

PersonalizationAppearanceContext::PersonalizationAppearanceContext(struct ::treeland_personalization_appearance_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationAppearanceContext>(1)
    , QtWayland::treeland_personalization_appearance_context_v1(context)
{
    get_round_corner_radius();
    get_font();
    get_monospace_font();
    get_cursor_theme();
    get_icon_theme();
}

int PersonalizationAppearanceContext::windowRadius() const
{
    return m_windowRadius;
}

void PersonalizationAppearanceContext::setWindowRadius(int radius)
{
    if (m_windowRadius == radius) {
        return;
    }

    m_windowRadius = radius;
    set_round_corner_radius(radius);
}

QString PersonalizationAppearanceContext::fontName() const
{
    return m_fontName;
}

void PersonalizationAppearanceContext::setFontName(const QString& fontName)
{
    if (m_fontName == fontName) {
        return;
    }

    m_fontName = fontName;
    set_font(fontName);
}

QString PersonalizationAppearanceContext::monoFontName() const
{
    return m_monoFontName;
}

void PersonalizationAppearanceContext::setMonoFontName(const QString& fontName)
{
    if (m_monoFontName == fontName) {
        return;
    }

    m_monoFontName = fontName;
    set_monospace_font(fontName);
}

QString PersonalizationAppearanceContext::cursorTheme() const
{
    return m_cursorTheme;
}

void PersonalizationAppearanceContext::setCursorTheme(const QString& cursorTheme)
{
    if (m_cursorTheme == cursorTheme) {
        return;
    }

    m_cursorTheme = cursorTheme;
    set_cursor_theme(cursorTheme);
}

QString PersonalizationAppearanceContext::iconTheme() const
{
    return m_iconTheme;
}

void PersonalizationAppearanceContext::setIconTheme(const QString& iconTheme)
{
    if (m_iconTheme == iconTheme) {
        return;
    }

    m_iconTheme = iconTheme;
    set_cursor_theme(iconTheme);
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_round_corner_radius(int32_t radius)
{
    m_windowRadius = radius;
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_font(const QString &font_name)
{
    m_fontName = font_name;
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_monospace_font(const QString &font_name)
{
    m_monoFontName = font_name;
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_cursor_theme(const QString &theme_name)
{
    m_cursorTheme = theme_name;
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_icon_theme(const QString &theme_name)
{
    m_iconTheme = theme_name;
}

PersonalizationWallpaperContext::PersonalizationWallpaperContext(struct ::treeland_personalization_wallpaper_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationWallpaperContext>(1)
    , QtWayland::treeland_personalization_wallpaper_context_v1(context)
{
}

PersonalizationCursorContext::PersonalizationCursorContext(struct ::treeland_personalization_cursor_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationCursorContext>(1)
    , QtWayland::treeland_personalization_cursor_context_v1(context)
{
}
DGUI_END_NAMESPACE
