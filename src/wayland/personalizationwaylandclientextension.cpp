// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "personalizationwaylandclientextension.h"
#include "qwayland-treeland-personalization-manager-v1.h"

#include <qwaylandclientextension.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

DGUI_BEGIN_NAMESPACE

class PersonalizationManager_: public PersonalizationManager {};
Q_GLOBAL_STATIC(PersonalizationManager_, personalizationManager)

PersonalizationManager::PersonalizationManager()
    : QWaylandClientExtensionTemplate<PersonalizationManager>(1)
{
}

PersonalizationManager *PersonalizationManager::instance()
{
    return personalizationManager;
}

PersonalizationWindowContext *PersonalizationManager::getWindowContext(QWindow *window)
{
    Q_ASSERT(!isActive());

    if (!window) {
        return nullptr;
    }
    window->create();
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (!waylandWindow) {
        return nullptr;
    }
    auto surface = waylandWindow->waylandSurface()->object();
    auto context = get_window_context(surface);
    if (m_windowContext.isNull()) {
        m_windowContext.reset(new PersonalizationWindowContext(context));
    }
    return m_windowContext.data();
}


bool PersonalizationManager::noTitlebar()
{
    return m_windowContext->noTitlebar();
}

void PersonalizationManager::setNoTitlebar(bool enable, QWindow *window)
{
    connect(this, &PersonalizationManager::activeChanged, [this, enable, window]{
        if (!isActive()) {
            return ;
        }

        auto windowContext = this->getWindowContext(window);
        if (!windowContext) {
            return;
        }
        windowContext->setNoTitlebar(enable);
    });
}

PersonalizationWindowContext::PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationWindowContext>(1)
    , QtWayland::treeland_personalization_window_context_v1(context)
{
}


bool PersonalizationWindowContext::noTitlebar()
{
    return m_noTitlebar;
}

void PersonalizationWindowContext::setNoTitlebar(bool enable)
{
    if (m_noTitlebar == enable) {
        return;
    }

    m_noTitlebar = enable;
    set_no_titlebar(m_noTitlebar ? enable_mode::enable_mode_enable : enable_mode::enable_mode_disable);
}

PersonalizationAppearanceContext::PersonalizationAppearanceContext(struct ::treeland_personalization_appearance_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationAppearanceContext>(1)
    , QtWayland::treeland_personalization_appearance_context_v1(context)
{
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
