// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplatforminterface.h"
#include "dplatforminterface_p.h"

#include <DGuiApplicationHelper>

#ifndef DTK_DISABLE_XCB
#include "platform/xcb/dxcbplatforminterface.h"
#endif

#ifndef DTK_DISABLE_TREELAND
#include "platform/treeland/dtreelandplatforminterface.h"
#endif


DGUI_BEGIN_NAMESPACE
DPlatformInterfacePrivate::DPlatformInterfacePrivate(DPlatformInterface *qq)
    : DObjectPrivate(qq)

{
}

DPlatformInterfacePrivate::~DPlatformInterfacePrivate()
{
}

DPlatformInterface::DPlatformInterface(QObject *parent)
    : DPlatformInterface(*new DPlatformInterfacePrivate(this), parent)
{
}

DPlatformInterface::~DPlatformInterface()
{
}

DPlatformInterface::DPlatformInterface(DPlatformInterfacePrivate &dd, QObject *parent)
    : QObject(parent)
    , DObject(dd)
{
}

bool DPlatformInterface::isEnabledNoTitlebar(QWindow *window) const
{
    return true;
}

bool DPlatformInterface::setEnabledNoTitlebar(QWindow *window, bool enable)
{
    return true;
}

int DPlatformInterface::windowRadius() const
{
    return 0;
}

void DPlatformInterface::setWindowRadius(int windowRadius)
{
}

QByteArray DPlatformInterface::fontName() const
{
    return "";
}

void DPlatformInterface::setFontName(const QByteArray &fontName)
{
}

QByteArray DPlatformInterface::monoFontName() const
{
    return "";
}

void DPlatformInterface::setMonoFontName(const QByteArray &monoFontName)
{
}

QByteArray DPlatformInterface::iconThemeName() const
{
    return "";
}

void DPlatformInterface::setIconThemeName(const QByteArray &iconThemeName)
{
}

QByteArray DPlatformInterface::cursorThemeName() const
{
    return "";
}

void DPlatformInterface::setCursorThemeName(const QByteArray &cursorThemeName)
{
}

DPlatformInterface* DPlatformInterface::self(QObject *parent)
{

#ifndef DTK_DISABLE_XCB
    if (DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsXWindowPlatform)) {
        return new DXCBPlatformInterface(0, parent);
    }
#endif

#ifndef DTK_DISABLE_TREELAND
    if (DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsTreelandPlatform)) {
       return new DTreelandPlatformInterface(parent);
    }
#endif

    return new DPlatformInterface(parent);
}


DGUI_END_NAMESPACE


