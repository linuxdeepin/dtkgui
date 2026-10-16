// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtreelandplatforminterface.h"

#include "dplatformtheme.h"
#include "private/dplatforminterface_p.h"
#include "treelandappearanceclientextension.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QWindow>

DGUI_BEGIN_NAMESPACE

DTreelandPlatformInterface::DTreelandPlatformInterface(DPlatformTheme *platformTheme)
    : DPlatformInterface(platformTheme)
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
    , m_blurOpacity(0.0)
{
    new TreelandAppearance(this);
}

DTreelandPlatformInterface::~DTreelandPlatformInterface() = default;

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

QColor DTreelandPlatformInterface::darkActiveColor() const
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
    Q_UNUSED(windowRadius)
}

void DTreelandPlatformInterface::setIconThemeName(const QByteArray &iconThemeName)
{
    Q_UNUSED(iconThemeName)
}

void DTreelandPlatformInterface::setFontName(const QByteArray &fontName)
{
    Q_UNUSED(fontName)
}

void DTreelandPlatformInterface::setMonoFontName(const QByteArray &monoFontName)
{
    Q_UNUSED(monoFontName)
}

void DTreelandPlatformInterface::setFontPointSize(qreal fontPointSize)
{
    Q_UNUSED(fontPointSize)
}

void DTreelandPlatformInterface::setActiveColor(const QColor activeColor)
{
    Q_UNUSED(activeColor)
}

DGUI_END_NAMESPACE
