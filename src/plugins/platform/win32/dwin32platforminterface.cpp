// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwin32platforminterface.h"

#include <QColor>

#include <qt_windows.h>

DGUI_BEGIN_NAMESPACE

static bool isSystemDarkMode()
{
    bool isDark = false;
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD value = 0;
        DWORD size = sizeof(value);
        if (RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr,
                            reinterpret_cast<LPBYTE>(&value), &size) == ERROR_SUCCESS) {
            isDark = (value == 0);
        }
        RegCloseKey(hKey);
    }
    return isDark;
}

DWin32PlatformInterface::DWin32PlatformInterface(DPlatformTheme *platformTheme)
    : DPlatformInterface(platformTheme)
{
}

QByteArray DWin32PlatformInterface::themeName() const
{
    return isSystemDarkMode() ? QByteArrayLiteral("dark") : QByteArrayLiteral("light");
}

QByteArray DWin32PlatformInterface::iconThemeName() const
{
    return m_iconThemeName;
}

QByteArray DWin32PlatformInterface::soundThemeName() const
{
    return {};
}

QByteArray DWin32PlatformInterface::fontName() const
{
    return m_fontName;
}

QByteArray DWin32PlatformInterface::monoFontName() const
{
    return m_monoFontName;
}

qreal DWin32PlatformInterface::fontPointSize() const
{
    return 0;
}

QByteArray DWin32PlatformInterface::gtkFontName() const
{
    return m_gtkFontName;
}

QColor DWin32PlatformInterface::activeColor() const
{
    return m_activeColor;
}

QColor DWin32PlatformInterface::darkActiveColor() const
{
    return m_darkActiveColor;
}

int DWin32PlatformInterface::windowRadius() const
{
    return -1;
}

int DWin32PlatformInterface::windowRadius(int defaultValue) const
{
    return defaultValue;
}

void DWin32PlatformInterface::setThemeName(const QByteArray &themeName)
{
    Q_UNUSED(themeName);
}

void DWin32PlatformInterface::setIconThemeName(const QByteArray &iconThemeName)
{
    m_iconThemeName = iconThemeName;
}

void DWin32PlatformInterface::setSoundThemeName(const QByteArray &soundThemeName)
{
    Q_UNUSED(soundThemeName);
}

void DWin32PlatformInterface::setFontName(const QByteArray &fontName)
{
    m_fontName = fontName;
}

void DWin32PlatformInterface::setMonoFontName(const QByteArray &monoFontName)
{
    m_monoFontName = monoFontName;
}

void DWin32PlatformInterface::setFontPointSize(qreal fontPointSize)
{
    Q_UNUSED(fontPointSize);
}

void DWin32PlatformInterface::setGtkFontName(const QByteArray &fontName)
{
    m_gtkFontName = fontName;
}

void DWin32PlatformInterface::setActiveColor(const QColor activeColor)
{
    m_activeColor = activeColor;
}

void DWin32PlatformInterface::setDarkActiveColor(const QColor &activeColor)
{
    m_darkActiveColor = activeColor;
}

void DWin32PlatformInterface::setWindowRadius(int windowRadius)
{
    Q_UNUSED(windowRadius);
}

DGUI_END_NAMESPACE
