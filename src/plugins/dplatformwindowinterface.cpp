// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "private/dplatformwindowinterface_p.h"
#include "private/dplatformwindowinterface_p_p.h"

DGUI_BEGIN_NAMESPACE

DPlatformWindowInterfacePrivate::DPlatformWindowInterfacePrivate(DPlatformWindowInterface *qq)
    : DObjectPrivate(qq)
{
}

DPlatformWindowInterface::DPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent)
    : DPlatformWindowInterface(*new DPlatformWindowInterfacePrivate(this), window, platformHandle, parent)
{
}

DPlatformWindowInterface::~DPlatformWindowInterface()
{
}

QWindow *DPlatformWindowInterface::window() const
{
    D_DC(DPlatformWindowInterface);

    return d->m_window;
}

bool DPlatformWindowInterface::setWindowBlurArea(const QVector<DPlatformHandle::WMBlurArea> &area)
{
    return {};
}

bool DPlatformWindowInterface::setWindowBlurArea(const QList<QPainterPath> &paths)
{
    return {};
}

bool DPlatformWindowInterface::setWindowWallpaperPara(const QRect &area, DPlatformHandle::WallpaperScaleMode sMode, DPlatformHandle::WallpaperFillMode fMode)
{
    return {};
}

bool DPlatformWindowInterface::isEnabledNoTitlebar() const
{
    return {};
}

bool DPlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    return {};
}

void DPlatformWindowInterface::setDisableWindowOverrideCursor(bool disable)
{
}

int DPlatformWindowInterface::windowRadius() const
{
    return {};
}

void DPlatformWindowInterface::setWindowRadius(int windowRadius)
{
}

int DPlatformWindowInterface::borderWidth() const
{
    return {};
}

void DPlatformWindowInterface::setBorderWidth(int borderWidth)
{
}

QColor DPlatformWindowInterface::borderColor() const
{
    return {};
}

void DPlatformWindowInterface::setBorderColor(const QColor &borderColor)
{
}

int DPlatformWindowInterface::shadowRadius() const
{
    return {};
}

void DPlatformWindowInterface::setShadowRadius(int shadowRadius)
{
}

QPoint DPlatformWindowInterface::shadowOffset() const
{
    return {};
}

void DPlatformWindowInterface::setShadowOffset(const QPoint &shadowOffset)
{
}

QColor DPlatformWindowInterface::shadowColor() const
{
    return {};
}

void DPlatformWindowInterface::setShadowColor(const QColor &shadowColor)
{
}

DPlatformHandle::EffectScene DPlatformWindowInterface::windowEffect()
{
    return {};
}

void DPlatformWindowInterface::setWindowEffect(DPlatformHandle::EffectScenes effectScene)
{
}

DPlatformHandle::EffectType DPlatformWindowInterface::windowStartUpEffect()
{
    return {};
}

void DPlatformWindowInterface::setWindowStartUpEffect(DPlatformHandle::EffectTypes effectType)
{
}

QPainterPath DPlatformWindowInterface::clipPath() const
{
    return {};
}

void DPlatformWindowInterface::setClipPath(const QPainterPath &clipPath)
{
}

QRegion DPlatformWindowInterface::frameMask() const
{
    return {};
}

void DPlatformWindowInterface::setFrameMask(const QRegion &frameMask)
{
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
}

bool DPlatformWindowInterface::enableSystemResize() const
{
    return {};
}

void DPlatformWindowInterface::setEnableSystemResize(bool enableSystemResize)
{
}

bool DPlatformWindowInterface::enableSystemMove() const
{
    return {};
}

void DPlatformWindowInterface::setEnableSystemMove(bool enableSystemMove)
{
}

bool DPlatformWindowInterface::enableBlurWindow() const
{
    return {};
}

void DPlatformWindowInterface::setEnableBlurWindow(bool enableBlurWindow)
{
}

bool DPlatformWindowInterface::autoInputMaskByClipPath() const
{
    return {};
}

void DPlatformWindowInterface::setAutoInputMaskByClipPath(bool autoInputMaskByClipPath)
{
}

WId DPlatformWindowInterface::realWindowId() const
{
    return {};
}

DPlatformWindowInterface::DPlatformWindowInterface(DPlatformWindowInterfacePrivate &dd, QWindow *window, DPlatformHandle *platformHandle, QObject *parent)
    : QObject(parent)
    , DObject(dd)
{
    d_func()->m_window = window;
    d_func()->m_platformHandle = platformHandle;
}

DGUI_END_NAMESPACE

