// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#include "dtkgui_global.h"
#define protected public
#include <QWindow>
#undef protected

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
    if (DVtableHook::hasVtable(m_window)) {
        DVtableHook::resetVtable(m_window);
    }

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
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::PlatformSurface) {
            QPlatformSurfaceEvent *se = static_cast<QPlatformSurfaceEvent*>(event);
            if (se->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                m_interface->doSetEnabledNoTitlebar();
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
}

DTreeLandPlatformWindowInterface::~DTreeLandPlatformWindowInterface()
{

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto waylandWindow = d->m_window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
#else
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(m_window->handle());
#endif
    if (!waylandWindow) {
        qWarning() << "waylandWindow is nullptr!!!";
        return nullptr;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto surface = waylandWindow->surface();
#else
    auto surface = waylandWindow->waylandSurface()->object();
#endif
    if (!surface) {
        qWarning() << "waylandSurface is nullptr!!!";
        return nullptr;
    }

    if (!d->m_windowContext) {
        d->m_windowContext =  new PersonalizationWindowContext(d->m_manager->get_window_context(surface));
    }

    connect(d->m_window, &QWindow::visibleChanged, d->m_windowContext, [this, d](bool visible){
        if (!visible) {
            d->m_windowContext->deleteLater();
            d->m_windowContext = nullptr;
        }
    });

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

bool DTreeLandPlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    D_D(DTreeLandPlatformWindowInterface);

    d->m_isNoTitlebar = enable;
    doSetEnabledNoTitlebar();
    return true;
}

void DTreeLandPlatformWindowInterface::setEnableBlurWindow(bool enable)
{
    D_D(DTreeLandPlatformWindowInterface);

    auto handleFunc = [this, enable](){
        auto windowContext = getWindowContext();
        if (!windowContext) {
            qWarning() << "windowContext is nullptr!";
            return;
        }
        windowContext->set_blend_mode(enable ? PersonalizationWindowContext::blend_mode_blur : PersonalizationWindowContext::blend_mode_transparent);
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

DGUI_END_NAMESPACE
