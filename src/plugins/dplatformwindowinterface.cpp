// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "private/dplatformwindowinterface_p.h"

DGUI_BEGIN_NAMESPACE

DPlatformWindowInterface::DPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle)
    : m_window(window)
    , m_platformHandle(platformHandle)
{
}

DPlatformWindowInterface::~DPlatformWindowInterface()
{
}

QWindow *DPlatformWindowInterface::window() const
{
    return m_window;
}

void DPlatformWindowInterface::setEnabled(bool enabled)
{
    Q_UNUSED(enabled)
}

bool DPlatformWindowInterface::isEnabled() const
{
    return false;
}

bool DPlatformWindowInterface::isEnabledNoTitlebar() const
{
    return {};
}

bool DPlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    Q_UNUSED(enable)
    return {};
}

void DPlatformWindowInterface::setDisableWindowOverrideCursor(bool disable)
{
    Q_UNUSED(disable)
}

int DPlatformWindowInterface::windowRadius() const
{
    return -1;
}

void DPlatformWindowInterface::setWindowRadius(int windowRadius)
{
    Q_UNUSED(windowRadius)
}

int DPlatformWindowInterface::borderWidth() const
{
    return {};
}

void DPlatformWindowInterface::setBorderWidth(int borderWidth)
{
    Q_UNUSED(borderWidth)
}

QColor DPlatformWindowInterface::borderColor() const
{
    return {};
}

void DPlatformWindowInterface::setBorderColor(const QColor &borderColor)
{
    Q_UNUSED(borderColor)
}

int DPlatformWindowInterface::shadowRadius() const
{
    return {};
}

void DPlatformWindowInterface::setShadowRadius(int shadowRadius)
{
    Q_UNUSED(shadowRadius)
}

QPoint DPlatformWindowInterface::shadowOffset() const
{
    return {};
}

void DPlatformWindowInterface::setShadowOffset(const QPoint &shadowOffset)
{
    Q_UNUSED(shadowOffset)
}

QColor DPlatformWindowInterface::shadowColor() const
{
    return {};
}

void DPlatformWindowInterface::setShadowColor(const QColor &shadowColor)
{
    Q_UNUSED(shadowColor)
}

DPlatformHandle::EffectScene DPlatformWindowInterface::windowEffect()
{
    return {};
}

void DPlatformWindowInterface::setWindowEffect(DPlatformHandle::EffectScenes effectScene)
{
    Q_UNUSED(effectScene)
}

DPlatformHandle::EffectType DPlatformWindowInterface::windowStartUpEffect()
{
    return {};
}

void DPlatformWindowInterface::setWindowStartUpEffect(DPlatformHandle::EffectTypes effectType)
{
    Q_UNUSED(effectType)
}

QPainterPath DPlatformWindowInterface::clipPath() const
{
    return {};
}

void DPlatformWindowInterface::setClipPath(const QPainterPath &clipPath)
{
    Q_UNUSED(clipPath)
}

QRegion DPlatformWindowInterface::frameMask() const
{
    return {};
}

void DPlatformWindowInterface::setFrameMask(const QRegion &frameMask)
{
    Q_UNUSED(frameMask)
}

QMargins DPlatformWindowInterface::frameMargins() const
{
    return {};
}

bool DPlatformWindowInterface::translucentBackground() const
{
    return {};
}

void DPlatformWindowInterface::setTranslucentBackground(bool translucentBackground)
{
    Q_UNUSED(translucentBackground)
}

bool DPlatformWindowInterface::enableSystemResize() const
{
    return {};
}

void DPlatformWindowInterface::setEnableSystemResize(bool enableSystemResize)
{
    Q_UNUSED(enableSystemResize)
}

bool DPlatformWindowInterface::enableSystemMove() const
{
    return {};
}

void DPlatformWindowInterface::setEnableSystemMove(bool enableSystemMove)
{
    Q_UNUSED(enableSystemMove)
}

bool DPlatformWindowInterface::enableBlurWindow() const
{
    return {};
}

void DPlatformWindowInterface::setEnableBlurWindow(bool enableBlurWindow)
{
    Q_UNUSED(enableBlurWindow)
}

DGUI_END_NAMESPACE

