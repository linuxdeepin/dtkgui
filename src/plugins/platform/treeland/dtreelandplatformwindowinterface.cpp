// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define protected public
#include <QWindow>
#undef protected

#include "dtkgui_global.h"
#include "plugins/platform/treeland/dtreelandplatforminterface.h"

#include "dtreelandplatformwindowinterface.h"
#include "dtreelandplatformwindowinterface_p.h"

#include <private/qwaylandintegration_p.h>
#include <private/qguiapplication_p.h>
#include <private/qwaylandwindow_p.h>

#include <qwaylandclientextension.h>
#include "dvtablehook.h"

#include <QtWaylandClient/QWaylandClientExtension>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QStyleHints>

DCORE_USE_NAMESPACE

class MoveWindowHelper : public QObject
{
public:
    explicit MoveWindowHelper(QWindow *w);
    ~MoveWindowHelper();

    static QHash<const QWindow*, MoveWindowHelper*> mapped;

private:
    static bool windowEvent(QWindow *w, QEvent *event);
    void updateEnableSystemMoveFromProperty();

    QWindow *m_window;
    bool m_windowMoving;
    bool m_enableSystemMove;
};

QHash<const QWindow*, MoveWindowHelper*> MoveWindowHelper::mapped;

MoveWindowHelper::MoveWindowHelper(QWindow *window)
    : QObject(window)
    , m_window(window)
{
    mapped[window] = this;
    updateEnableSystemMoveFromProperty();
}

MoveWindowHelper::~MoveWindowHelper()
{
    mapped.remove(qobject_cast<QWindow*>(parent()));
}

void MoveWindowHelper::updateEnableSystemMoveFromProperty()
{
    if (!m_window) {
        return;
    }
    const QVariant &v = m_window->property("_d_enableSystemMove");

    m_enableSystemMove = !v.isValid() || v.toBool();

    if (m_enableSystemMove) {
        DVtableHook::overrideVfptrFun(m_window, &QWindow::event, &MoveWindowHelper::windowEvent);
    } else if (DVtableHook::hasVtable(m_window)) {
        DVtableHook::resetVfptrFun(m_window, &QWindow::event);
    }
}

bool MoveWindowHelper::windowEvent(QWindow *w, QEvent *event)
{
    MoveWindowHelper *self = mapped.value(w);

    if (!self)
        return DVtableHook::callOriginalFun(w, &QWindow::event, event);

    // TODO Crashed when delete by Vtable.
    if (event->type() == QEvent::DeferredDelete) {
        DVtableHook::resetVtable(w);
        return w->event(event);
    }

    // m_window 的 event 被 override 以后，在 windowEvent 里面获取到的 this 就成 m_window 了，
    // 而不是 DNoTitlebarWlWindowHelper，所以此处 windowEvent 改为 static 并传 self 进来
    {
        static bool isTouchDown = false;
        static QPointF touchBeginPosition;
        if (event->type() == QEvent::TouchBegin) {
            isTouchDown = true;
        }
        if (event->type() == QEvent::TouchEnd || event->type() == QEvent::MouseButtonRelease) {
            isTouchDown = false;
        }
        if (isTouchDown && event->type() == QEvent::MouseButtonPress) {
            touchBeginPosition = static_cast<QMouseEvent*>(event)->globalPos();
        }
        // add some redundancy to distinguish trigger between system menu and system move
        if (event->type() == QEvent::MouseMove) {
            QPointF currentPos = static_cast<QMouseEvent*>(event)->globalPos();
            QPointF delta = touchBeginPosition  - currentPos;
            if (delta.manhattanLength() < QGuiApplication::styleHints()->startDragDistance()) {
                return DVtableHook::callOriginalFun(w, &QWindow::event, event);
            }
        }
    }

    bool is_mouse_move = event->type() == QEvent::MouseMove && static_cast<QMouseEvent*>(event)->buttons() == Qt::LeftButton;

    if (event->type() == QEvent::MouseButtonRelease) {
        self->m_windowMoving = false;
    }

    if (!DVtableHook::callOriginalFun(w, &QWindow::event, event))
        return false;

    // workaround for kwin: Qt receives no release event when kwin finishes MOVE operation,
    // which makes app hang in windowMoving state. when a press happens, there's no sense of
    // keeping the moving state, we can just reset ti back to normal.
    if (event->type() == QEvent::MouseButtonPress) {
        self->m_windowMoving = false;
    }

    if (is_mouse_move && !event->isAccepted()
            && w->geometry().contains(static_cast<QMouseEvent*>(event)->globalPos())) {
        if (!self->m_windowMoving && self->m_enableSystemMove) {
            self->m_windowMoving = true;

            event->accept();
            if (w && w->handle()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                static_cast<QPlatformWindow *>(w->handle())->startSystemMove(QCursor::pos());
#else
                static_cast<QPlatformWindow *>(w->handle())->startSystemMove();
#endif
    }
        }
    }

    return true;
}

class Q_DECL_HIDDEN WindowEventFilter : public QObject {
public:
    WindowEventFilter(QObject *parent = nullptr, DTreeLandPlatformWindowInterface *interface = nullptr)
        : QObject(parent)
        , m_interface(interface)
    {
    }

public:
    bool eventFilter(QObject *watched, QEvent *event) {
        if (event->type() == QEvent::PlatformSurface) {
            QPlatformSurfaceEvent *se = static_cast<QPlatformSurfaceEvent*>(event);
            if (se->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                m_interface->initWaylandWindow();
                m_interface->onSurfaceCreated();
            }
        }
        return QObject::eventFilter(watched, event);
    }
private:
    DTreeLandPlatformWindowInterface *m_interface;
};

DGUI_BEGIN_NAMESPACE

DTreeLandPlatformWindowInterfacePrivate::DTreeLandPlatformWindowInterfacePrivate(DTreeLandPlatformWindowInterface *qq)
    : DPlatformWindowInterfacePrivate(qq)
{
}

DTreeLandPlatformWindowInterface::DTreeLandPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent)
    : DPlatformWindowInterface(*new DTreeLandPlatformWindowInterfacePrivate(this), window, platformHandle, parent)
{
    D_D(DTreeLandPlatformWindowInterface);

    d->m_manager = PersonalizationManager::instance();
    d->m_window->installEventFilter(new WindowEventFilter(this, this));
    connect(d->m_manager, &PersonalizationManager::activeChanged, this, [this, d](){
        if (d->m_manager->isActive()) {
            handlePendingTasks();
        }
    });

    if (!MoveWindowHelper::mapped.value(window)) {
        Q_UNUSED(new MoveWindowHelper(window))
    }

    initWaylandWindow();
}

DTreeLandPlatformWindowInterface::~DTreeLandPlatformWindowInterface()
{
}

void DTreeLandPlatformWindowInterface::onSurfaceCreated()
{
    D_D(DTreeLandPlatformWindowInterface);

    if (d->m_isNoTitlebar) {
        doSetEnabledNoTitlebar();
    }
    if (d->m_isWindowBlur) {
        doSetEnabledBlurWindow();
    }
}

void DTreeLandPlatformWindowInterface::onSurfaceDestroyed()
{
    D_D(DTreeLandPlatformWindowInterface);

    if (d->m_windowContext) {
        d->m_windowContext->deleteLater();
        d->m_windowContext = nullptr;
    }
}

void DTreeLandPlatformWindowInterface::initWaylandWindow()
{
    D_D(DTreeLandPlatformWindowInterface);

    // force create window handle
    d->m_window->winId();

    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(d->m_window->handle());

    if (!waylandWindow) {
        qWarning() << "waylandWindow is nullptr!!!";
        return;
    }

    connect(waylandWindow, &QtWaylandClient::QWaylandWindow::wlSurfaceCreated, this, &DTreeLandPlatformWindowInterface::onSurfaceCreated, Qt::UniqueConnection);
    connect(waylandWindow, &QtWaylandClient::QWaylandWindow::wlSurfaceDestroyed, this, &DTreeLandPlatformWindowInterface::onSurfaceDestroyed, Qt::UniqueConnection);
}

PersonalizationWindowContext *DTreeLandPlatformWindowInterface::getWindowContext()
{
    D_D(DTreeLandPlatformWindowInterface);

    if (!d->m_manager->isSupported()) {
        return nullptr;
    }
    if (!d->m_window) {
        qWarning() << "window is nullptr!!!";
        return nullptr;
    }
    if (d->m_windowContext) {
        return d->m_windowContext;
    }
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(d->m_window->handle());
    if (!waylandWindow) {
        qWarning() << "waylandWindow is nullptr!!!";
        return nullptr;
    }

    auto surface = waylandWindow->waylandSurface()->object();
    if (!surface) {
        qWarning() << "waylandSurface is nullptr!!!";
        return nullptr;
    }

    if (!d->m_windowContext) {
        d->m_windowContext =  new PersonalizationWindowContext(d->m_manager->get_window_context(surface));
    }

    return d->m_windowContext;
}

void DTreeLandPlatformWindowInterface::handlePendingTasks()
{
    D_D(DTreeLandPlatformWindowInterface);

    while (!d->m_pendingTasks.isEmpty()) {
        auto handleFunc = d->m_pendingTasks.dequeue();
        handleFunc();
    }
}

bool DTreeLandPlatformWindowInterface::isEnabledNoTitlebar() const
{
    D_DC(DTreeLandPlatformWindowInterface);

    return d->m_isNoTitlebar;
}

bool DTreeLandPlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    D_D(DTreeLandPlatformWindowInterface);

    if (d->m_isNoTitlebar == enable) {
        return true;
    }
    d->m_isNoTitlebar = enable;
    doSetEnabledNoTitlebar();
    return true;
}

bool DTreeLandPlatformWindowInterface::setWindowBlurArea(const QVector<DPlatformHandle::WMBlurArea> &area)
{
    qWarning("the interface (setWindowBlurArea) is not currently supported in Treeland");
    return false;
}

bool DTreeLandPlatformWindowInterface::setWindowBlurArea(const QList<QPainterPath> &paths)
{
    qWarning("the interface (setWindowBlurArea) is not currently supported in Treeland");
    return false;
}

bool DTreeLandPlatformWindowInterface::setWindowWallpaperPara(const QRect &area, DPlatformHandle::WallpaperScaleMode sMode, DPlatformHandle::WallpaperFillMode fMode)
{
    qWarning("the interface (setWindowWallpaperPara) is not currently supported in Treeland");
    return false;
}

void DTreeLandPlatformWindowInterface::setDisableWindowOverrideCursor(bool disable)
{
    qWarning("the interface (setWindowBlurArea) is not currently supported in Treeland");
    return;
}

int DTreeLandPlatformWindowInterface::windowRadius() const
{
    D_DC(DTreeLandPlatformWindowInterface);

    return d->m_radius;
}

void DTreeLandPlatformWindowInterface::setWindowRadius(int windowRadius)
{
    D_D(DTreeLandPlatformWindowInterface);

    if (d->m_radius == windowRadius) {
        return;
    }
    d->m_radius = windowRadius;
    doSetWindowRadius();
}

int DTreeLandPlatformWindowInterface::borderWidth() const
{
    qWarning("the interface (borderWidth) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setBorderWidth(int borderWidth)
{
    qWarning("the interface (setBorderWidth) is not currently supported in Treeland");
    return;
}

QColor DTreeLandPlatformWindowInterface::borderColor() const
{
    qWarning("the interface (borderColor) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setBorderColor(const QColor &borderColor)
{
    qWarning("the interface (setBorderColor) is not currently supported in Treeland");
    return;
}

int DTreeLandPlatformWindowInterface::shadowRadius() const
{
    qWarning("the interface (shadowRadius) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setShadowRadius(int shadowRadius)
{
    qWarning("the interface (setShadowRadius) is not currently supported in Treeland");
    return;
}

QPoint DTreeLandPlatformWindowInterface::shadowOffset() const
{
    qWarning("the interface (shadowOffset) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setShadowOffset(const QPoint &shadowOffset)
{
    qWarning("the interface (setShadowOffset) is not currently supported in Treeland");
    return;
}

QColor DTreeLandPlatformWindowInterface::shadowColor() const
{
    qWarning("the interface (shadowColor) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setShadowColor(const QColor &shadowColor)
{
    qWarning("the interface (setShadowColor) is not currently supported in Treeland");
    return;
}

DPlatformHandle::EffectScene DTreeLandPlatformWindowInterface::windowEffect()
{
    qWarning("the interface (windowEffect) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setWindowEffect(DPlatformHandle::EffectScenes effectScene)
{
    qWarning("the interface (setWindowEffect) is not currently supported in Treeland");
    return;
}

DPlatformHandle::EffectType DTreeLandPlatformWindowInterface::windowStartUpEffect()
{
    qWarning("the interface (windowStartUpEffect) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setWindowStartUpEffect(DPlatformHandle::EffectTypes effectType)
{
    qWarning("the interface (setWindowStartUpEffect) is not currently supported in Treeland");
    return;
}

QPainterPath DTreeLandPlatformWindowInterface::clipPath() const
{
    qWarning("the interface (clipPath) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setClipPath(const QPainterPath &clipPath)
{
    qWarning("the interface (setClipPath) is not currently supported in Treeland");
    return;
}

QRegion DTreeLandPlatformWindowInterface::frameMask() const
{
    qWarning("the interface (frameMask) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setFrameMask(const QRegion &frameMask)
{
    qWarning("the interface (setFrameMask) is not currently supported in Treeland");
    return;
}

QMargins DTreeLandPlatformWindowInterface::frameMargins() const
{
    qWarning("the interface (frameMargins) is not currently supported in Treeland");
    return {};
}

bool DTreeLandPlatformWindowInterface::translucentBackground() const
{
    qWarning("the interface (translucentBackground) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setTranslucentBackground(bool translucentBackground)
{
    qWarning("the interface (setTranslucentBackground) is not currently supported in Treeland");
    return;
}

bool DTreeLandPlatformWindowInterface::enableSystemResize() const
{
    qWarning("the interface (enableSystemResize) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setEnableSystemResize(bool enableSystemResize)
{
    qWarning("the interface (setWindowWallpaperPara) is not currently supported in Treeland");
    return;
}

bool DTreeLandPlatformWindowInterface::enableSystemMove() const
{
    qWarning("the interface (enableSystemMove) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setEnableSystemMove(bool enableSystemMove)
{
    qWarning("the interface (setEnableSystemMove) is not currently supported in Treeland");
    return;
}

bool DTreeLandPlatformWindowInterface::enableBlurWindow() const
{
    qWarning("the interface (enableBlurWindow) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setEnableBlurWindow(bool enable)
{
    D_D(DTreeLandPlatformWindowInterface);

    if (d->m_isWindowBlur == enable) {
        return;
    }

    d->m_isWindowBlur = enable;
    doSetEnabledBlurWindow();
}

bool DTreeLandPlatformWindowInterface::autoInputMaskByClipPath() const
{
    qWarning("the interface (autoInputMaskByClipPath) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::setAutoInputMaskByClipPath(bool autoInputMaskByClipPath)
{
    qWarning("the interface (setAutoInputMaskByClipPath) is not currently supported in Treeland");
    return;
}

WId DTreeLandPlatformWindowInterface::realWindowId() const
{
    qWarning("the interface (realWindowId) is not currently supported in Treeland");
    return {};
}

void DTreeLandPlatformWindowInterface::doSetEnabledBlurWindow()
{
    D_D(DTreeLandPlatformWindowInterface);

    auto handleFunc = [this, d](){
        auto windowContext = getWindowContext();
        if (!windowContext) {
            qWarning() << "windowContext is nullptr!";
            return;
        }
        windowContext->set_blend_mode(d->m_isWindowBlur ? PersonalizationWindowContext::blend_mode_blur : PersonalizationWindowContext::blend_mode_transparent);
    };
    if (d->m_manager->isActive()) {
        handleFunc();
    } else {
        d->m_pendingTasks.enqueue(handleFunc);
    }
}

void DTreeLandPlatformWindowInterface::doSetEnabledNoTitlebar()
{
    D_D(DTreeLandPlatformWindowInterface);

    auto handleFunc = [this, d](){
        auto windowContext = getWindowContext();
        if (!windowContext) {
            qWarning() << "windowContext is nullptr!";
            return false;
        }
        windowContext->set_titlebar(d->m_isNoTitlebar ? PersonalizationWindowContext::enable_mode_disable : PersonalizationWindowContext::enable_mode_enable);
        return true;
    };
    if (d->m_manager->isActive()) {
        handleFunc();
    } else {
        d->m_pendingTasks.enqueue(handleFunc);
    }
}

void DTreeLandPlatformWindowInterface::doSetWindowRadius()
{
    D_D(DTreeLandPlatformWindowInterface);

    auto handleFunc = [this, d] (){
        auto windowContext = getWindowContext();
        if (!windowContext) {
            qWarning() << "windowContext is nullptr!";
            return false;
        }
        windowContext->set_round_corner_radius(d->m_radius);
        return true;
    };
    if (d->m_manager->isActive()) {
        handleFunc();
    } else {
        d->m_pendingTasks.enqueue(handleFunc);
    }
}
DGUI_END_NAMESPACE
