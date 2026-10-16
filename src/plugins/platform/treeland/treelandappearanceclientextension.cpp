// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "treelandappearanceclientextension.h"

#include "dplatformtheme.h"

DGUI_BEGIN_NAMESPACE

TreelandAppearance::TreelandAppearance(DTreelandPlatformInterface *interface)
    : QWaylandClientExtensionTemplate<TreelandAppearance>(treeland_appearance_v1_interface.version)
    , m_interface(interface)
{
    setParent(interface);
}

TreelandAppearance::~TreelandAppearance()
{
    if (isInitialized())
        destroy();
}

void TreelandAppearance::treeland_appearance_v1_cursor_theme(const QString &name)
{
    Q_UNUSED(name)
    // dtkgui currently has no public API for cursor theme; cache only if needed in future
}

void TreelandAppearance::treeland_appearance_v1_cursor_size(uint32_t width, uint32_t height)
{
    Q_UNUSED(width)
    Q_UNUSED(height)
    // dtkgui currently has no public API for cursor size
}

void TreelandAppearance::treeland_appearance_v1_font(const QString &font_name)
{
    if (m_interface) {
        m_interface->m_fontName = font_name.toUtf8();
        emit m_interface->m_platformTheme->fontNameChanged(font_name.toUtf8());
    }
}

void TreelandAppearance::treeland_appearance_v1_monospace_font(const QString &font_name)
{
    if (m_interface) {
        m_interface->m_monoFontName = font_name.toUtf8();
        emit m_interface->m_platformTheme->monoFontNameChanged(font_name.toUtf8());
    }
}

void TreelandAppearance::treeland_appearance_v1_font_size(uint32_t size)
{
    if (m_interface) {
        // Treeland stores the point size multiplied by 10 to preserve fractional sizes.
        qreal pointSize = size / 10.0;
        pointSize = qBound(8.25, pointSize, 15.00);
        m_interface->m_fontPointSize = pointSize;
        emit m_interface->m_platformTheme->fontPointSizeChanged(pointSize);
    }
}

void TreelandAppearance::treeland_appearance_v1_icon_theme(const QString &theme_name)
{
    if (m_interface) {
        m_interface->m_iconThemeName = theme_name.toUtf8();
        emit m_interface->m_platformTheme->iconThemeNameChanged(theme_name.toUtf8());
    }
}

void TreelandAppearance::treeland_appearance_v1_accent_color(uint32_t r, uint32_t g, uint32_t b)
{
    if (m_interface) {
        m_interface->m_activeColor = QColor(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b));
        emit m_interface->m_platformTheme->activeColorChanged(m_interface->m_activeColor);
    }
}

void TreelandAppearance::treeland_appearance_v1_window_opacity(wl_fixed_t opacity)
{
    if (m_interface) {
        m_interface->m_blurOpacity = wl_fixed_to_double(opacity);
    }
}

void TreelandAppearance::treeland_appearance_v1_color_scheme(uint32_t scheme)
{
    if (m_interface) {
        QString theme = QStringLiteral("deepin");
        if (scheme == static_cast<uint32_t>(color_scheme_dark)) {
            theme = theme + QStringLiteral("-dark");
        }
        m_interface->m_themeName = theme.toUtf8();
        emit m_interface->m_platformTheme->themeNameChanged(theme.toUtf8());
    }
}

void TreelandAppearance::treeland_appearance_v1_titlebar_height(uint32_t height)
{
    if (m_interface) {
        m_interface->m_titleHeight = static_cast<int>(height);
    }
}

void TreelandAppearance::treeland_appearance_v1_corner_radius(uint32_t radius)
{
    if (m_interface) {
        m_interface->m_windowRadius = static_cast<int>(radius);
        emit m_interface->m_platformTheme->windowRadiusChanged(static_cast<int>(radius));
    }
}

DGUI_END_NAMESPACE
