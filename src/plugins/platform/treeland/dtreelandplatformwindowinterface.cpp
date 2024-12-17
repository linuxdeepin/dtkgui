// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define protected public
#include <QWindow>
#undef protected

#include "dtreelandplatformwindowinterface.h"

#include <QWaylandClientExtension>
#include <QStyleHints>
#include <private/qwaylandintegration_p.h>
#include <private/qguiapplication_p.h>
#include <private/qwaylandwindow_p.h>
#include <private/qwaylandsurface_p.h>

#include "dvtablehook.h"

#include "personalizationwaylandclientextension.h"

DCORE_USE_NAMESPACE

DGUI_BEGIN_NAMESPACE
class Q_DECL_HIDDEN MoveWindowHelper : public QObject
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

QMap<QWindow *, DTreeLandPlatformWindowHelper*> DTreeLandPlatformWindowHelper::windowMap;
DTreeLandPlatformWindowHelper *DTreeLandPlatformWindowHelper::get(QWindow *window) 
{
    if (!PersonalizationManager::instance()->isSupported()) {
        return nullptr;
    }

    if (!window) {
        return nullptr;
    }

    if (auto helper = windowMap.value(window)) {
        return helper;
    }
    auto helper = new DTreeLandPlatformWindowHelper(window);
    windowMap[window] = helper;
    return helper;
}

DTreeLandPlatformWindowHelper::DTreeLandPlatformWindowHelper(QWindow *window)
    : QObject(window)
{
    window->installEventFilter(this);

    if (!PersonalizationManager::instance()->isActive()) {
        qWarning() << "Personalization is not active" << window;
        connect(PersonalizationManager::instance(), &PersonalizationManager::activeChanged, this, &DTreeLandPlatformWindowHelper::onActiveChanged, Qt::QueuedConnection);
    }

    if (window->handle()) {
        initWaylandWindow();
    }
}

DTreeLandPlatformWindowHelper::~DTreeLandPlatformWindowHelper()
{
    windowMap.remove(window());
}

bool DTreeLandPlatformWindowHelper::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::PlatformSurface) {
        QPlatformSurfaceEvent *se = static_cast<QPlatformSurfaceEvent*>(event);
        if (se->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
            if (PersonalizationManager::instance()->isActive()) {
                initWaylandWindow();
                onSurfaceCreated();
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

void DTreeLandPlatformWindowHelper::initWaylandWindow()
{
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window()->handle());
    if (!waylandWindow) {
        qWarning() << "waylandWindow is nullptr!!!";
        return;
    }

    connect(waylandWindow, &QtWaylandClient::QWaylandWindow::wlSurfaceCreated, this, &DTreeLandPlatformWindowHelper::onSurfaceCreated, Qt::UniqueConnection);
    connect(waylandWindow, &QtWaylandClient::QWaylandWindow::wlSurfaceDestroyed, this, &DTreeLandPlatformWindowHelper::onSurfaceDestroyed, Qt::UniqueConnection);
}

void DTreeLandPlatformWindowHelper::onActiveChanged()
{
    if (PersonalizationManager::instance()->isActive()) {
        qDebug() << "Personalization is actived, window" << window();
        if (window()->handle()) {
            onSurfaceCreated();
        }
    }
}

void DTreeLandPlatformWindowHelper::onSurfaceCreated()
{
    Q_EMIT surfaceCreated();
}

void DTreeLandPlatformWindowHelper::onSurfaceDestroyed()
{
    if (m_windowContext) {
        m_windowContext->deleteLater();
        m_windowContext = nullptr;
    }
}

PersonalizationWindowContext *DTreeLandPlatformWindowHelper::windowContext() const
{
    if (m_windowContext) {
        return m_windowContext;
    }

    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window()->handle());
    if (!waylandWindow)
        return nullptr;

    if (!waylandWindow->waylandSurface()) {
        qWarning() << "waylandSurface is nullptr!!!";
        return nullptr;
    }

    auto surface = waylandWindow->waylandSurface()->object();
    if (!surface) {
        qWarning() << "wl_surface is nullptr!!!";
        return nullptr;
    }

    if (!m_windowContext) {
        const_cast<DTreeLandPlatformWindowHelper *>(this)->m_windowContext = new PersonalizationWindowContext(PersonalizationManager::instance()->get_window_context(surface));
    }

    return m_windowContext;
}

DTreeLandPlatformWindowInterface::DTreeLandPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent)
    : QObject(parent)
    , DPlatformWindowInterface(window, platformHandle)
{
    if (!MoveWindowHelper::mapped.value(window)) {
        Q_UNUSED(new MoveWindowHelper(window))
    }
    
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        connect(helper, &DTreeLandPlatformWindowHelper::surfaceCreated, this, &DTreeLandPlatformWindowInterface::onSurfaceCreated);
    }
}

DTreeLandPlatformWindowInterface::~DTreeLandPlatformWindowInterface()
{
}

void DTreeLandPlatformWindowInterface::onSurfaceCreated()
{
    if (m_isNoTitlebar) {
        doSetEnabledNoTitlebar();
    }
    if (m_isWindowBlur) {
        doSetEnabledBlurWindow();
    }
}

void DTreeLandPlatformWindowInterface::setEnabled(bool enabled)
{
    if (setEnabledNoTitlebar(enabled)) {
        return;
    }
}

bool DTreeLandPlatformWindowInterface::isEnabled() const
{
    return isEnabledNoTitlebar();
}

bool DTreeLandPlatformWindowInterface::isEnabledNoTitlebar() const
{
    return m_isNoTitlebar;
}

bool DTreeLandPlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    if (m_isNoTitlebar == enable) {
        return true;
    }
    m_isNoTitlebar = enable;
    doSetEnabledNoTitlebar();
    return true;
}

int DTreeLandPlatformWindowInterface::windowRadius() const
{
    return m_radius;
}

void DTreeLandPlatformWindowInterface::setWindowRadius(int windowRadius)
{
    if (m_radius == windowRadius) {
        return;
    }
    m_radius = windowRadius;
    doSetWindowRadius();
}

bool DTreeLandPlatformWindowInterface::enableBlurWindow() const
{
    return m_isWindowBlur;
}

void DTreeLandPlatformWindowInterface::setEnableBlurWindow(bool enable)
{
    if (m_isWindowBlur == enable) {
        return;
    }
    m_isWindowBlur = enable;
    doSetEnabledBlurWindow();
}

void DTreeLandPlatformWindowInterface::doSetEnabledNoTitlebar()
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        auto context = helper->windowContext();
        if (!context) {
            return;
        }
        context->set_titlebar(m_isNoTitlebar ? PersonalizationWindowContext::enable_mode_disable : PersonalizationWindowContext::enable_mode_enable);
    }
}

void DTreeLandPlatformWindowInterface::doSetWindowRadius()
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        auto context = helper->windowContext();
        if (!context) {
            return;
        }
        context->set_round_corner_radius(m_radius);
        if (m_platformHandle) {
            Q_EMIT m_platformHandle->windowRadiusChanged();
        }
    }
}

void DTreeLandPlatformWindowInterface::doSetEnabledBlurWindow()
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        auto context = helper->windowContext();
        if (!context) {
            return;
        }
        context->set_blend_mode(m_isWindowBlur ? PersonalizationWindowContext::blend_mode_blur : PersonalizationWindowContext::blend_mode_transparent);
        if (m_platformHandle) {
            Q_EMIT m_platformHandle->enableBlurWindowChanged();
        }
    }
}
DGUI_END_NAMESPACE
