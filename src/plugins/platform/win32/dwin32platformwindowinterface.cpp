// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwin32platformwindowinterface.h"
#include "dguiapplicationhelper.h"

#include <QGuiApplication>
#include <QCoreApplication>
#include <QWindow>
#include <QMouseEvent>
#include <qpa/qplatformwindow.h>
#include <QStyleHints>
#include <QSettings>
#include <QTimer>
#include <QAbstractNativeEventFilter>
#include <QOperatingSystemVersion>

#include <qt_windows.h>
#include <dwmapi.h>

// DWMWA_WINDOW_CORNER_PREFERENCE (Win 11 build 22000+)
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif

#ifndef DWMWCP_DONOTROUND
#define DWMWCP_DONOTROUND 1
#endif

// DWMWA_USE_IMMERSIVE_DARK_MODE (Win 10 17763+)
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

// DWMWA_MICA_EFFECT (Win 11 22000+)
#ifndef DWMWA_MICA_EFFECT
#define DWMWA_MICA_EFFECT 1029
#endif

static HWND getHwnd(QWindow *window)
{
    if (!window)
        return nullptr;
    return reinterpret_cast<HWND>(window->winId());
}

DGUI_BEGIN_NAMESPACE

DWin32PlatformWindowInterface::DWin32PlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent)
    : QObject(parent)
    , DPlatformWindowInterface(window, platformHandle)
{
    if (m_window) {
        m_window->installEventFilter(this);
    }
}

DWin32PlatformWindowInterface::~DWin32PlatformWindowInterface()
{
    if (m_window) {
        m_window->removeEventFilter(this);
    }
}

void *DWin32PlatformWindowInterface::hwnd() const
{
    return getHwnd(m_window);
}

void DWin32PlatformWindowInterface::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    setEnabledNoTitlebar(enabled);
}

bool DWin32PlatformWindowInterface::isEnabled() const
{
    return m_enabled;
}

bool DWin32PlatformWindowInterface::isEnabledNoTitlebar() const
{
    return m_enabled;
}

bool DWin32PlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    HWND h = getHwnd(m_window);
    if (!h)
        return false;

    // Remove border/frame styles for frameless look
    LONG style = GetWindowLongPtrW(h, GWL_STYLE);
    if (enable) {
        style &= ~(WS_CAPTION | WS_THICKFRAME | WS_SYSMENU);
    } else {
        style |= WS_CAPTION | WS_THICKFRAME;
    }
    SetWindowLongPtrW(h, GWL_STYLE, style);

    // Apply style change and force redraw
    RECT rc;
    GetWindowRect(h, &rc);
    SetWindowPos(h, HWND_TOP, rc.left, rc.top,
                 rc.right - rc.left, rc.bottom - rc.top,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE);
    return true;
}

void DWin32PlatformWindowInterface::setDisableWindowOverrideCursor(bool disable)
{
    if (m_disableOverrideCursor == disable)
        return;

    m_disableOverrideCursor = disable;

    // On Windows, we can prevent cursor changes by setting SetCursor in response
    // to WM_SETCURSOR. This is handled at a higher level.
}

int DWin32PlatformWindowInterface::windowRadius() const
{
    return m_windowRadius;
}

void DWin32PlatformWindowInterface::setWindowRadius(int windowRadius)
{
    if (m_windowRadius == windowRadius)
        return;

    m_windowRadius = windowRadius;

    // Defer to showEvent if window not yet visible
    if (m_windowVisible)
        updateRoundedCorners();

    if (m_platformHandle)
        Q_EMIT m_platformHandle->windowRadiusChanged();
}

int DWin32PlatformWindowInterface::borderWidth() const
{
    return m_borderWidth;
}

void DWin32PlatformWindowInterface::setBorderWidth(int borderWidth)
{
    if (m_borderWidth == borderWidth)
        return;

    m_borderWidth = borderWidth;

    if (m_platformHandle)
        Q_EMIT m_platformHandle->borderWidthChanged();
}

QColor DWin32PlatformWindowInterface::borderColor() const
{
    return m_borderColor;
}

void DWin32PlatformWindowInterface::setBorderColor(const QColor &borderColor)
{
    if (m_borderColor == borderColor)
        return;

    m_borderColor = borderColor;

    if (m_platformHandle)
        Q_EMIT m_platformHandle->borderColorChanged();
}

int DWin32PlatformWindowInterface::shadowRadius() const
{
    return m_shadowRadius;
}

void DWin32PlatformWindowInterface::setShadowRadius(int shadowRadius)
{
    if (m_shadowRadius == shadowRadius)
        return;

    m_shadowRadius = shadowRadius;

    if (m_platformHandle)
        Q_EMIT m_platformHandle->shadowRadiusChanged();
}

QPoint DWin32PlatformWindowInterface::shadowOffset() const
{
    return m_shadowOffset;
}

void DWin32PlatformWindowInterface::setShadowOffset(const QPoint &shadowOffset)
{
    if (m_shadowOffset == shadowOffset)
        return;

    m_shadowOffset = shadowOffset;

    if (m_platformHandle)
        Q_EMIT m_platformHandle->shadowOffsetChanged();
}

QColor DWin32PlatformWindowInterface::shadowColor() const
{
    return m_shadowColor;
}

void DWin32PlatformWindowInterface::setShadowColor(const QColor &shadowColor)
{
    if (m_shadowColor == shadowColor)
        return;

    m_shadowColor = shadowColor;

    if (m_platformHandle)
        Q_EMIT m_platformHandle->shadowColorChanged();
}

DPlatformHandle::EffectScene DWin32PlatformWindowInterface::windowEffect()
{
    return static_cast<DPlatformHandle::EffectScene>(m_windowEffect.toInt());
}

void DWin32PlatformWindowInterface::setWindowEffect(DPlatformHandle::EffectScenes effectScene)
{
    if (m_windowEffect == effectScene)
        return;

    m_windowEffect = effectScene;

    HWND h = getHwnd(m_window);
    if (!h)
        return;

    // Apply effect using DWM attributes
    BOOL disableRoundCorner = effectScene.testFlag(DPlatformHandle::EffectNoRadius);
    DwmSetWindowAttribute(h, DWMWA_WINDOW_CORNER_PREFERENCE,
                          &disableRoundCorner, sizeof(disableRoundCorner));

    if (m_platformHandle)
        Q_EMIT m_platformHandle->windowEffectChanged();
}

DPlatformHandle::EffectType DWin32PlatformWindowInterface::windowStartUpEffect()
{
    return static_cast<DPlatformHandle::EffectType>(m_windowStartUpEffect.toInt());
}

void DWin32PlatformWindowInterface::setWindowStartUpEffect(DPlatformHandle::EffectTypes effectType)
{
    if (m_windowStartUpEffect == effectType)
        return;

    m_windowStartUpEffect = effectType;

    // Startup effects are typically animation hints that may not have direct
    // Win32 equivalents. Store the value for potential use by the application.
    if (m_platformHandle)
        Q_EMIT m_platformHandle->windowStartUpEffectChanged();
}

QPainterPath DWin32PlatformWindowInterface::clipPath() const
{
    return m_clipPath;
}

void DWin32PlatformWindowInterface::setClipPath(const QPainterPath &clipPath)
{
    if (m_clipPath == clipPath)
        return;

    m_clipPath = clipPath;

    if (m_windowVisible)
        updateClipPath();

    if (m_platformHandle)
        Q_EMIT m_platformHandle->clipPathChanged();
}

QRegion DWin32PlatformWindowInterface::frameMask() const
{
    return m_frameMask;
}

void DWin32PlatformWindowInterface::setFrameMask(const QRegion &frameMask)
{
    if (m_frameMask == frameMask)
        return;

    m_frameMask = frameMask;

    if (m_windowVisible)
        updateFrameMask();

    if (m_platformHandle)
        Q_EMIT m_platformHandle->frameMaskChanged();
}

QMargins DWin32PlatformWindowInterface::frameMargins() const
{
    HWND h = getHwnd(m_window);
    if (!h)
        return {};

    // Get extended frame bounds which accounts for DWM rendering
    RECT frameRect = {};
    HRESULT hr = DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS,
                                       &frameRect, sizeof(frameRect));
    if (FAILED(hr))
        return {};

    RECT windowRect = {};
    GetWindowRect(h, &windowRect);

    return QMargins(
        frameRect.left - windowRect.left,
        frameRect.top - windowRect.top,
        windowRect.right - frameRect.right,
        windowRect.bottom - frameRect.bottom
    );
}

bool DWin32PlatformWindowInterface::translucentBackground() const
{
    return m_translucentBackground;
}

void DWin32PlatformWindowInterface::setTranslucentBackground(bool translucentBackground)
{
    if (m_translucentBackground == translucentBackground)
        return;

    m_translucentBackground = translucentBackground;

    if (m_windowVisible)
        updateTranslucentBackground();

    if (m_platformHandle)
        Q_EMIT m_platformHandle->translucentBackgroundChanged();
}

bool DWin32PlatformWindowInterface::enableSystemResize() const
{
    return m_enableSystemResize;
}

void DWin32PlatformWindowInterface::setEnableSystemResize(bool enableSystemResize)
{
    if (m_enableSystemResize == enableSystemResize)
        return;

    m_enableSystemResize = enableSystemResize;

    HWND h = getHwnd(m_window);
    if (h) {
        LONG style = GetWindowLongPtrW(h, GWL_STYLE);
        if (enableSystemResize) {
            style |= WS_THICKFRAME;
        } else {
            style &= ~WS_THICKFRAME;
        }
        SetWindowLongPtrW(h, GWL_STYLE, style);

        RECT rc;
        GetWindowRect(h, &rc);
        SetWindowPos(h, HWND_TOP, rc.left, rc.top,
                     rc.right - rc.left, rc.bottom - rc.top,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);
    }

    if (m_platformHandle)
        Q_EMIT m_platformHandle->enableSystemResizeChanged();
}

bool DWin32PlatformWindowInterface::enableSystemMove() const
{
    return m_enableSystemMove;
}

void DWin32PlatformWindowInterface::setEnableSystemMove(bool enableSystemMove)
{
    if (m_enableSystemMove == enableSystemMove)
        return;

    m_enableSystemMove = enableSystemMove;

    if (m_platformHandle)
        Q_EMIT m_platformHandle->enableSystemMoveChanged();
}

bool DWin32PlatformWindowInterface::enableBlurWindow() const
{
    return m_enableBlurWindow;
}

void DWin32PlatformWindowInterface::setEnableBlurWindow(bool enableBlurWindow)
{
    if (m_enableBlurWindow == enableBlurWindow)
        return;

    m_enableBlurWindow = enableBlurWindow;

    if (m_windowVisible)
        updateBlurWindow();

    if (m_platformHandle)
        Q_EMIT m_platformHandle->enableBlurWindowChanged();
}

bool DWin32PlatformWindowInterface::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_window)
        return QObject::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::Show:
        m_windowVisible = true;
        setEnabledNoTitlebar(m_enabled);
        updateShadow();
        updateRoundedCorners();
        updateSystemTheme();
        updateClipPath();
        updateFrameMask();
        updateTranslucentBackground();
        updateBlurWindow();
        break;
    case QEvent::PlatformSurface:
        break;
    case QEvent::MouseButtonPress: {
        if (!m_enableSystemMove)
            break;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            // Only allow drag from title bar area (top 50px)
            const int titleBarHeight = 50;
            if (me->position().y() < titleBarHeight) {
                m_pressPoint = me->globalPosition();
                m_windowMoving = false;
            } else {
                m_pressPoint = QPointF();
            }
        }
        break;
    }
    case QEvent::MouseMove: {
        if (!m_enableSystemMove || m_pressPoint.isNull())
            break;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->buttons() != Qt::LeftButton)
            break;

        QPointF delta = me->globalPosition() - m_pressPoint;
        if (!m_windowMoving) {
            if (delta.manhattanLength() < QGuiApplication::styleHints()->startDragDistance())
                break;
            m_windowMoving = true;
        }

        if (m_window && m_window->handle()) {
            m_window->handle()->startSystemMove();
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        m_windowMoving = false;
        m_pressPoint = QPointF();
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}

void DWin32PlatformWindowInterface::updateSystemTheme()
{
    if (QOperatingSystemVersion::current().microVersion() < 17763)
        return;

    HWND h = getHwnd(m_window);
    if (!h)
        return;

    // Detect system theme and apply immersive dark mode if available
    bool isDark = false;

    // Check registry for system theme
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

    BOOL useDarkMode = isDark;
    DwmSetWindowAttribute(h, DWMWA_USE_IMMERSIVE_DARK_MODE,
                          &useDarkMode, sizeof(useDarkMode));

    m_systemDarkMode = isDark;
}

void DWin32PlatformWindowInterface::updateShadow()
{
    HWND h = getHwnd(m_window);
    if (!h)
        return;

    // DwmExtendFrameIntoClientArea with {-1,-1,-1,-1} would make the entire
    // window glass/transparent - not what we want. Skip it for now.
    // A proper shadow implementation would use DWMWA_NCRENDERING_POLICY or
    // a custom shadow surface.
}

void DWin32PlatformWindowInterface::updateRoundedCorners()
{
    HWND h = getHwnd(m_window);
    if (!h)
        return;

    if (QOperatingSystemVersion::current().microVersion() < 22000)
        return;

    int preference = (m_windowRadius > 0) ? DWMWCP_ROUND : DWMWCP_DONOTROUND;
    HRESULT hr = DwmSetWindowAttribute(h, DWMWA_WINDOW_CORNER_PREFERENCE,
                                       &preference, sizeof(preference));
    if (FAILED(hr)) {
        qWarning() << "DwmSetWindowAttribute(DWMWA_WINDOW_CORNER_PREFERENCE) failed:" << QString::number(hr, 16);
    }
}

void DWin32PlatformWindowInterface::updateClipPath()
{
    HWND h = getHwnd(m_window);
    if (!h)
        return;

    if (m_clipPath.isEmpty()) {
        // Reset to full window region
        SetWindowRgn(h, nullptr, TRUE);
        return;
    }

    // Convert QPainterPath to Windows region
    QList<QPolygonF> polygons = m_clipPath.toSubpathPolygons();
    if (polygons.isEmpty())
        return;

    // Create region from path polygons
    HRGN hRgn = nullptr;
    for (const QPolygonF &polygon : polygons) {
        QPolygon poly = polygon.toPolygon();
        if (poly.isEmpty())
            continue;

        // Convert QPolygon to POINT array
        QVector<POINT> points(poly.size());
        for (int i = 0; i < poly.size(); ++i) {
            points[i].x = poly[i].x();
            points[i].y = poly[i].y();
        }

        HRGN hPolyRgn = CreatePolygonRgn(points.constData(), points.size(), WINDING);
        if (hPolyRgn) {
            if (hRgn) {
                HRGN hCombined = CreateRectRgn(0, 0, 0, 0);
                CombineRgn(hCombined, hRgn, hPolyRgn, RGN_OR);
                DeleteObject(hRgn);
                DeleteObject(hPolyRgn);
                hRgn = hCombined;
            } else {
                hRgn = hPolyRgn;
            }
        }
    }

    if (hRgn) {
        SetWindowRgn(h, hRgn, TRUE);
        // Windows takes ownership of the region, don't delete it
    }
}

void DWin32PlatformWindowInterface::updateFrameMask()
{
    HWND h = getHwnd(m_window);
    if (!h)
        return;

    if (m_frameMask.isEmpty()) {
        // Reset to full window region
        SetWindowRgn(h, nullptr, TRUE);
        return;
    }

    // Create region from rectangles
    HRGN hRgn = nullptr;
    for (const QRect &rect : m_frameMask) {
        HRGN hRectRgn = CreateRectRgn(rect.left(), rect.top(),
                                       rect.right(), rect.bottom());
        if (hRectRgn) {
            if (hRgn) {
                HRGN hCombined = CreateRectRgn(0, 0, 0, 0);
                CombineRgn(hCombined, hRgn, hRectRgn, RGN_OR);
                DeleteObject(hRgn);
                DeleteObject(hRectRgn);
                hRgn = hCombined;
            } else {
                hRgn = hRectRgn;
            }
        }
    }

    if (hRgn) {
        SetWindowRgn(h, hRgn, TRUE);
        // Windows takes ownership of the region, don't delete it
    }
}

void DWin32PlatformWindowInterface::updateTranslucentBackground()
{
    HWND h = getHwnd(m_window);
    if (!h)
        return;

    if (m_translucentBackground) {
        // Enable layered window for transparency
        LONG exStyle = GetWindowLongPtrW(h, GWL_EXSTYLE);
        exStyle |= WS_EX_LAYERED;
        SetWindowLongPtrW(h, GWL_EXSTYLE, exStyle);

        // Set opacity to fully opaque (transparency handled by paint)
        SetLayeredWindowAttributes(h, 0, 255, LWA_ALPHA);

        // Extend frame into client area for glass effect
        MARGINS margins = {-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(h, &margins);
    } else {
        // Remove layered style
        LONG exStyle = GetWindowLongPtrW(h, GWL_EXSTYLE);
        exStyle &= ~WS_EX_LAYERED;
        SetWindowLongPtrW(h, GWL_EXSTYLE, exStyle);
    }
}

void DWin32PlatformWindowInterface::updateBlurWindow()
{
    HWND h = getHwnd(m_window);
    if (!h)
        return;

    if (m_enableBlurWindow) {
        // Enable blur behind window using DWM
        DWM_BLURBEHIND bb = {};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = TRUE;
        bb.hRgnBlur = nullptr;
        DwmEnableBlurBehindWindow(h, &bb);

        // Extend frame into client area for glass effect
        MARGINS margins = {-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(h, &margins);
    } else {
        // Disable blur
        DWM_BLURBEHIND bb = {};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = FALSE;
        DwmEnableBlurBehindWindow(h, &bb);

        // Reset margins
        MARGINS margins = {0, 0, 0, 0};
        DwmExtendFrameIntoClientArea(h, &margins);
    }
}

DGUI_END_NAMESPACE
