// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dxcbplatforminterface.h"
#include "dxcbplatforminterface_p.h"

#include <DNativeSettings>

#include <QGuiApplication>
#include <QPlatformSurfaceEvent>
#include <qpa/qplatformwindow.h>

#include "dplatformtheme.h"
#include "dguiapplicationhelper.h"

DGUI_BEGIN_NAMESPACE

#define DEFINE_CONST_CHAR(Name) const char _##Name[] = "_d_" #Name

DEFINE_CONST_CHAR(setEnableNoTitlebar);
DEFINE_CONST_CHAR(isEnableNoTitlebar);
DEFINE_CONST_CHAR(windowRadius);
DEFINE_CONST_CHAR(setWindowProperty);
DEFINE_CONST_CHAR(resolve_mask);

enum PropRole {
    WindowRadius,

    // TO BE CONTINUE
};

#define FETCH_PROPERTY(Name, Function) \
    D_DC(DXCBPlatformInterface); \
    QVariant value = d->m_nativeSettings->getSetting(QByteArrayLiteral(Name)); \
    if (d->fallbackProperty && !value.isValid() && d->parent) \
        return d->parent->Function(); \

static bool resolved(QObject *obj, PropRole role)
{
    int mask = obj->property(_resolve_mask).toInt();
    return mask & (1 << role);
}

static void setWindowProperty(QWindow *window, const char *name, const QVariant &value)
{
    if (!window)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    static QFunctionPointer setWindowProperty = qApp->platformFunction(_setWindowProperty);
#else
    constexpr QFunctionPointer setWindowProperty = nullptr;
#endif

    if (!setWindowProperty) {
        window->setProperty(name, value);

        return;
    }

    reinterpret_cast<void(*)(QWindow *, const char *, const QVariant &)>(setWindowProperty)(window, name, value);
}

static void initWindowRadius(QWindow *window)
{
    if (window->property(_windowRadius).isValid())
        return;

    auto theme = DGuiApplicationHelper::instance()->systemTheme();
    int radius = theme->windowRadius(18); //###(zccrs): 暂时在此处给窗口默认设置为18px的圆角

    setWindowProperty(window, _windowRadius, radius);
    // Qt::UniqueConnection will report a warning
    // to `unique connections require a pointer to member function of a QObject subclass`.
    const char *uniqueueConnectionFlag("_d_uniqueueConnectionFlag");
    bool connected = window->property(uniqueueConnectionFlag).toBool();
    if (!connected) {
        window->setProperty(uniqueueConnectionFlag, true);
        window->connect(theme, &DPlatformTheme::windowRadiusChanged, window, [window] (int radius) {
            if (!resolved(window, PropRole::WindowRadius))
                setWindowProperty(window, _windowRadius, radius);
        });
    }
}

class Q_DECL_HIDDEN CreatorWindowEventFile : public QObject
{
    bool m_windowMoving = false;
public:
    CreatorWindowEventFile(QObject *par= nullptr): QObject(par){}

public:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::PlatformSurface) {
            QPlatformSurfaceEvent *se = static_cast<QPlatformSurfaceEvent*>(event);
            if (se->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {  // 若收到此信号， 则 WinID 已被创建
                initWindowRadius(qobject_cast<QWindow *>(watched));
                deleteLater();
            }
        }

        if (auto *w = qobject_cast<QWindow *>(watched); w && DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsTreelandPlatform)) {
            bool is_mouse_move = event->type() == QEvent::MouseMove && static_cast<QMouseEvent*>(event)->buttons() == Qt::LeftButton;

            if (event->type() == QEvent::MouseButtonRelease) {
                m_windowMoving = false;
            }

            // workaround for kwin: Qt receives no release event when kwin finishes MOVE operation,
            // which makes app hang in windowMoving state. when a press happens, there's no sense of
            // keeping the moving state, we can just reset ti back to normal.
            if (event->type() == QEvent::MouseButtonPress) {
                m_windowMoving = false;
            }

            // FIXME: We need to check whether the event is accepted.
            //        Only when the upper control does not accept the event,
            //        the window should be moved through the window.
            //        But every event here has been accepted. I don't know what happened.
            if (is_mouse_move && w->geometry().contains(static_cast<QMouseEvent*>(event)->globalPos())) {
                if (!m_windowMoving) {
                    m_windowMoving = true;

                    event->accept();
                    static_cast<QPlatformWindow *>(w->handle())->startSystemMove();
                }
            }
        }

        return QObject::eventFilter(watched, event);
    }
};

DXCBPlatformInterfacePrivate::DXCBPlatformInterfacePrivate(DXCBPlatformInterface *qq)
    : DPlatformInterfacePrivate(qq)
{
}

DXCBPlatformInterface::DXCBPlatformInterface(quint32 window, QObject *parent)
    : DPlatformInterface(*new DXCBPlatformInterfacePrivate(this), parent)
{
    D_D(DXCBPlatformInterface);

    d->m_nativeSettings = new DNativeSettings(window, QByteArray(), this);
}

bool DXCBPlatformInterface::isEnabledNoTitlebar(QWindow *window) const
{
    QFunctionPointer is_enable_no_titlebar = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    is_enable_no_titlebar = qApp->platformFunction(_isEnableNoTitlebar);
#endif

    if (is_enable_no_titlebar) {
        return (*reinterpret_cast<bool(*)(const QWindow*)>(is_enable_no_titlebar))(window);
    }

    return false;
}

bool DXCBPlatformInterface::setEnabledNoTitlebar(QWindow *window, bool enable)
{
    if (isEnabledNoTitlebar(window) == enable)
        return true;

    QFunctionPointer enable_no_titlear = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    enable_no_titlear = qApp->platformFunction(_setEnableNoTitlebar);
#endif

    if (enable_no_titlear) {
        bool ok = (*reinterpret_cast<bool(*)(QWindow*, bool)>(enable_no_titlear))(window, enable);
        if (ok && enable) {
            if (window->handle()) {
                initWindowRadius(window);
            } else {
                window->installEventFilter(new CreatorWindowEventFile(window));
            }
        }

        return ok;
    }

    return false;

}

int DXCBPlatformInterface::windowRadius() const
{
    FETCH_PROPERTY("DTK/WindowRadius", windowRadius);
    return value.toInt();
}

void DXCBPlatformInterface::setWindowRadius(int windowRadius)
{
    D_D(DXCBPlatformInterface);
    d->m_nativeSettings->setSetting("DTK/WindowRadius", windowRadius);
}

QByteArray DXCBPlatformInterface::fontName() const
{
    FETCH_PROPERTY("Qt/FontName", fontName)
    return value.toByteArray();
}

void DXCBPlatformInterface::setFontName(const QByteArray &fontName)
{
    D_D(DXCBPlatformInterface);
    d->m_nativeSettings->setSetting("Qt/FontName", fontName);
}

QByteArray DXCBPlatformInterface::monoFontName() const
{
    FETCH_PROPERTY("Qt/MonoFontName", monoFontName)
    return value.toByteArray();
}

void DXCBPlatformInterface::setMonoFontName(const QByteArray &monoFontName)
{
    D_D(DXCBPlatformInterface);
    d->m_nativeSettings->setSetting("Qt/MonoFontName", monoFontName);
}

QByteArray DXCBPlatformInterface::iconThemeName() const
{
    FETCH_PROPERTY("Net/IconThemeName", iconThemeName)
    return value.toByteArray();
}

void DXCBPlatformInterface::setIconThemeName(const QByteArray &iconThemeName)
{
    D_D(DXCBPlatformInterface);
    d->m_nativeSettings->setSetting("Net/IconThemeName", iconThemeName);
}

QByteArray DXCBPlatformInterface::cursorThemeName() const
{
    FETCH_PROPERTY("Gtk/CursorThemeName", cursorThemeName)
    return value.toByteArray();
}

void DXCBPlatformInterface::setCursorThemeName(const QByteArray &cursorThemeName)
{
    D_D(DXCBPlatformInterface);
    d->m_nativeSettings->setSetting("Gtk/CursorThemeName", cursorThemeName);
}

DGUI_END_NAMESPACE
