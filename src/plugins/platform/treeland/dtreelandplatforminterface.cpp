// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtreelandplatforminterface.h"

#include "dplatformtheme.h"
#include "private/dplatforminterface_p.h"
#include "personalizationwaylandclientextension.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QWindow>

DGUI_BEGIN_NAMESPACE

DTreelandPlatformInterface::DTreelandPlatformInterface(DPlatformTheme *platformTheme)
    : DPlatformInterface(platformTheme)
    , m_manager(nullptr)
    , m_activeColor(QColor())
    , m_titleHeight(0)
    , m_fontName(QByteArray())
    , m_monoFontName(QByteArray())
    , m_iconThemeName(QByteArray())
    , m_cursorThemeName(QByteArray())
    , m_fontPointSize(0.0)
    , m_windowRadius(0)
    , m_scrollBarPolicy(0)
    , m_themeName(QByteArray())
    , m_blurOpacity(0)
{
    m_manager = PersonalizationManager::instance();
    connect(m_manager, &PersonalizationManager::activeChanged, this, [this](){
        if (m_manager->isActive()) {
            initContext();
        }
    });
}

void DTreelandPlatformInterface::initContext()
{
    if (m_appearanceContext.isNull()) { 
        m_appearanceContext.reset(new PersonalizationAppearanceContext(m_manager->get_appearance_context(), this));
    }
    if (m_fontContext.isNull()) {
        m_fontContext.reset(new PersonalizationFontContext(m_manager->get_font_context(), this));
    }
}

QByteArray DTreelandPlatformInterface::iconThemeName() const
{
    return m_iconThemeName;
}

QByteArray DTreelandPlatformInterface::fontName() const
{
    return m_fontName;
}

QByteArray DTreelandPlatformInterface::monoFontName() const
{
    return m_monoFontName;
}

qreal DTreelandPlatformInterface::fontPointSize() const
{
    return m_fontPointSize;
}

QColor DTreelandPlatformInterface::activeColor() const
{
    return m_activeColor;
}

QByteArray DTreelandPlatformInterface::themeName() const
{
    return m_themeName;
}

int DTreelandPlatformInterface::windowRadius() const
{
    return m_windowRadius;
}

int DTreelandPlatformInterface::windowRadius(int defaultValue) const
{
    return m_windowRadius > 0 ? m_windowRadius : defaultValue;
}

void DTreelandPlatformInterface::setWindowRadius(int windowRadius)
{
    if (m_appearanceContext) {
        m_appearanceContext->set_round_corner_radius(windowRadius);
    }
}

void DTreelandPlatformInterface::setIconThemeName(const QByteArray &iconThemeName)
{
    if (m_appearanceContext) {
        m_appearanceContext->set_icon_theme(iconThemeName);
    }
}

void DTreelandPlatformInterface::setFontName(const QByteArray &fontName)
{
    if (m_fontContext) {
        m_fontContext->set_font(fontName);
    }
}

void DTreelandPlatformInterface::setMonoFontName(const QByteArray &monoFontName)
{
    if (m_fontContext) {
        m_fontContext->set_monospace_font(monoFontName);
    }
}

void DTreelandPlatformInterface::setFontPointSize(qreal fontPointSize)
{
    if (m_fontContext) {
        m_fontContext->set_font_size(fontPointSize);
    }
}

void DTreelandPlatformInterface::setActiveColor(const QColor activeColor)
{
    if (m_appearanceContext) {
        m_appearanceContext->set_active_color(activeColor.name());
    }
}

DGUI_END_NAMESPACE
