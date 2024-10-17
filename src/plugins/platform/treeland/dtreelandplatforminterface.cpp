// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtreelandplatforminterface.h"
#include "dtreelandplatforminterface_p.h"
#include "personalizationwaylandclientextension.h"

DGUI_BEGIN_NAMESPACE
DTreelandPlatformInterfacePrivate::DTreelandPlatformInterfacePrivate(DTreelandPlatformInterface *qq)
    : DPlatformInterfacePrivate(qq)
{
}

DTreelandPlatformInterface::DTreelandPlatformInterface(QObject *parent)
    : DPlatformInterface(*new DTreelandPlatformInterfacePrivate(this), parent)
{
    D_D(DTreelandPlatformInterface);
}

bool DTreelandPlatformInterface::isEnabledNoTitlebar(QWindow *window) const
{
    return PersonalizationManager::instance()->isEnabledNoTitlebar(window);
}

bool DTreelandPlatformInterface::setEnabledNoTitlebar(QWindow *window, bool enable)
{
    return PersonalizationManager::instance()->setEnabledNoTitlebar(window, enable);
}

int DTreelandPlatformInterface::windowRadius() const
{
    return PersonalizationManager::instance()->windowRadius();
}

void DTreelandPlatformInterface::setWindowRadius(int windowRadius)
{
    PersonalizationManager::instance()->setWindowRadius(windowRadius);
}

QByteArray DTreelandPlatformInterface::fontName() const
{
    return PersonalizationManager::instance()->fontName().toUtf8();
}

void DTreelandPlatformInterface::setFontName(const QByteArray &fontName)
{
    PersonalizationManager::instance()->setFontName(fontName);
}

QByteArray DTreelandPlatformInterface::monoFontName() const
{
    return PersonalizationManager::instance()->monoFontName().toUtf8();
}

void DTreelandPlatformInterface::setMonoFontName(const QByteArray &monoFontName)
{
    PersonalizationManager::instance()->setMonoFontName(monoFontName);
}

QByteArray DTreelandPlatformInterface::iconThemeName() const
{
    return PersonalizationManager::instance()->iconThemeName().toUtf8();
}

void DTreelandPlatformInterface::setIconThemeName(const QByteArray &iconThemeName)
{
    PersonalizationManager::instance()->setIconThemeName(iconThemeName);
}

QByteArray DTreelandPlatformInterface::cursorThemeName() const
{
    return PersonalizationManager::instance()->cursorThemeName().toUtf8();
}

void DTreelandPlatformInterface::setCursorThemeName(const QByteArray &cursorThemeName)
{
    PersonalizationManager::instance()->setCursorThemeName(cursorThemeName);
}

DGUI_END_NAMESPACE
