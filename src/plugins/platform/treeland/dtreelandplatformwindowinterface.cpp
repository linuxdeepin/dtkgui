// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QWindow>

#include "dtreelandplatformwindowinterface.h"
#include "util/dprivateaccessor_p.h"

#include <QWaylandClientExtension>
#include <QStyleHints>
#include <private/qwaylandintegration_p.h>
#include <private/qguiapplication_p.h>
#include <private/qwaylandwindow_p.h>
#include <private/qwaylandsurface_p.h>

#include "dvtablehook.h"

#include "personalizationwaylandclientextension.h"

DCORE_USE_NAMESPACE

D_DECLARE_PRIVATE_METHOD(QWindow_event_tag, QWindow, event, bool, QEvent *);

DGUI_BEGIN_NAMESPACE

static inline auto qWindowEventMember()
{
    return get(QWindow_event_tag{});
}

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
    QPointF m_pressPoint;
};

QHash<const QWindow*, MoveWindowHelper*> MoveWindowHelper::mapped;

MoveWindowHelper::MoveWindowHelper(QWindow *window)
    : QObject(window)
    , m_window(window)
    , m_windowMoving(false)
    , m_enableSystemMove(false)
{
    mapped[window] = this;
    updateEnableSystemMoveFromProperty();
}

MoveWindowHelper::~MoveWindowHelper()
{
    mapped.remove(static_cast<QWindow*>(parent()));
}

void MoveWindowHelper::updateEnableSystemMoveFromProperty()
{
    if (!m_window) {
        return;
    }
    const QVariant &v = m_window->property("_d_enableSystemMove");

    m_enableSystemMove = !v.isValid() || v.toBool();

    if (m_enableSystemMove) {
        DVtableHook::overrideVfptrFun(m_window, qWindowEventMember(), &MoveWindowHelper::windowEvent);
    } else if (DVtableHook::hasVtable(m_window)) {
        m_windowMoving = false;
        m_pressPoint = QPointF();
        DVtableHook::resetVfptrFun(m_window, qWindowEventMember());
    }
}

bool MoveWindowHelper::windowEvent(QWindow *w, QEvent *event)
{
    MoveWindowHelper *self = mapped.value(w);

    if (!self)
        return DVtableHook::callOriginalFun(w, qWindowEventMember(), event);

    // TODO Crashed when delete by Vtable.
    if (event->type() == QEvent::DeferredDelete) {
        DVtableHook::resetVtable(w);
        return D_PRIVATE_CALL(*w, QWindow_event_tag{}, event);
    }

    // get touch begin position
    static bool isTouchDown = false;
    static QPointF touchBeginPosition;

    if (event->type() == QEvent::TouchBegin) {
        isTouchDown = true;
    }
    if (event->type() == QEvent::TouchEnd || event->type() == QEvent::MouseButtonRelease) {
        isTouchDown = false;
    }
    if (isTouchDown && event->type() == QEvent::MouseButtonPress) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        touchBeginPosition = static_cast<QMouseEvent*>(event)->globalPosition();
#else
        touchBeginPosition = static_cast<QMouseEvent*>(event)->globalPos();
#endif
    }
    // add some redundancy to distinguish trigger between system menu and system move
    if (event->type() == QEvent::MouseMove) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPointF currentPos = static_cast<QMouseEvent*>(event)->globalPosition();
#else
        QPointF currentPos = static_cast<QMouseEvent*>(event)->globalPos();
#endif
        QPointF delta = touchBeginPosition - currentPos;
        if (delta.manhattanLength() < QGuiApplication::styleHints()->startDragDistance()) {
            return DVtableHook::callOriginalFun(w, qWindowEventMember(), event);
        }
    }

    bool is_mouse_move = event->type() == QEvent::MouseMove
        && static_cast<QMouseEvent*>(event)->buttons() == Qt::LeftButton;

    if (event->type() == QEvent::MouseButtonRelease) {
        self->m_windowMoving = false;
        self->m_pressPoint = QPointF();
    }

    bool ret = DVtableHook::callOriginalFun(w, qWindowEventMember(), event);

    // workaround for kwin: Qt receives no release event when kwin finishes MOVE operation,
    // which makes app hang in windowMoving state. when a press happens, there's no sense of
    // keeping the moving state, we can just reset it back to normal.
    if (event->type() == QEvent::MouseButtonPress) {
        self->m_windowMoving = false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        self->m_pressPoint = static_cast<QMouseEvent*>(event)->globalPosition();
#else
        self->m_pressPoint = static_cast<QMouseEvent*>(event)->globalPos();
#endif
    }

    if (is_mouse_move && !event->isAccepted() && !self->m_pressPoint.isNull()) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        QRect windowRect = QRect(QPoint(0, 0), w->size());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (!windowRect.contains(me->scenePosition().toPoint())) {
#else
        if (!windowRect.contains(me->localPos().toPoint())) {
#endif
            return ret;
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPointF delta = me->globalPosition() - self->m_pressPoint;
#else
        QPointF delta = me->globalPos() - self->m_pressPoint;
#endif
        if (delta.manhattanLength() < QGuiApplication::styleHints()->startDragDistance()) {
            return ret;
        }

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

    return ret;
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
    if (m_windowContext) {
        m_windowContext->deleteLater();
        m_windowContext = nullptr;
    }
    windowMap.remove(static_cast<QWindow*>(parent()));
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
    m_dirty = m_initialized;
    scheduleApply();
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

void DTreeLandPlatformWindowHelper::setEnabledNoTitlebar(bool enable)
{
    updateFeature(m_noTitlebar, enable, NoTitlebar);
}

void DTreeLandPlatformWindowHelper::setWindowRadius(int windowRadius)
{
    updateFeature(m_radius, windowRadius, Radius);
}

void DTreeLandPlatformWindowHelper::setBorderWidth(int borderWidth)
{
    updateFeature(m_borderWidth, borderWidth, Border);
}

void DTreeLandPlatformWindowHelper::setBorderColor(const QColor &borderColor)
{
    updateFeature(m_borderColor, borderColor, Border);
}

void DTreeLandPlatformWindowHelper::setShadowRadius(int shadowRadius)
{
    updateFeature(m_shadowRadius, shadowRadius, Shadow);
}

void DTreeLandPlatformWindowHelper::setShadowOffset(const QPoint &shadowOffset)
{
    updateFeature(m_shadowOffset, shadowOffset, Shadow);
}

void DTreeLandPlatformWindowHelper::setShadowColor(const QColor &shadowColor)
{
    updateFeature(m_shadowColor, shadowColor, Shadow);
}

void DTreeLandPlatformWindowHelper::setEnableBlurWindow(bool enableBlurWindow)
{
    updateFeature(m_blur, enableBlurWindow, Blur);
}

void DTreeLandPlatformWindowHelper::setPlatformHandle(DPlatformHandle *handle)
{
    m_platformHandle = handle;
}

void DTreeLandPlatformWindowHelper::scheduleApply()
{
    if (m_applyScheduled)
        return;
    m_applyScheduled = true;
    QMetaObject::invokeMethod(this, &DTreeLandPlatformWindowHelper::applyPending);
}

void DTreeLandPlatformWindowHelper::applyPending()
{
    m_applyScheduled = false;

    if (auto context = windowContext()) {
        if (m_dirty & NoTitlebar) {
            m_dirty &= ~NoTitlebar;
            context->set_titlebar(m_noTitlebar ? PersonalizationWindowContext::enable_mode_disable : PersonalizationWindowContext::enable_mode_enable);
        }
        if (m_dirty & Radius) {
            m_dirty &= ~Radius;
            context->set_round_corner_radius(m_radius);
        }
        if (m_dirty & Blur) {
            m_dirty &= ~Blur;
            context->set_blend_mode(m_blur ? PersonalizationWindowContext::blend_mode_blur : PersonalizationWindowContext::blend_mode_transparent);
        }
        if (m_dirty & Border) {
            m_dirty &= ~Border;
            context->set_border(m_borderWidth,
                                m_borderColor.red(), m_borderColor.green(),
                                m_borderColor.blue(), m_borderColor.alpha());
        }
        if (m_dirty & Shadow) {
            m_dirty &= ~Shadow;
            context->set_shadow(m_shadowRadius,
                                m_shadowOffset.x(), m_shadowOffset.y(),
                                m_shadowColor.red(), m_shadowColor.green(),
                                m_shadowColor.blue(), m_shadowColor.alpha());
        }
    }
}

DTreeLandPlatformWindowInterface::DTreeLandPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent)
    : QObject(parent)
    , DPlatformWindowInterface(window, platformHandle)
{
    if (!MoveWindowHelper::mapped.value(window)) {
        Q_UNUSED(new MoveWindowHelper(window))
    }
    if (auto helper = DTreeLandPlatformWindowHelper::get(window)) {
        helper->setPlatformHandle(platformHandle);
    }
}

DTreeLandPlatformWindowInterface::~DTreeLandPlatformWindowInterface()
{
}

void DTreeLandPlatformWindowInterface::setEnabled(bool enabled)
{
    setEnabledNoTitlebar(enabled);
}

bool DTreeLandPlatformWindowInterface::isEnabled() const
{
    return isEnabledNoTitlebar();
}

bool DTreeLandPlatformWindowInterface::isEnabledNoTitlebar() const
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->isEnabledNoTitlebar();
    return false;
}

bool DTreeLandPlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setEnabledNoTitlebar(enable);
        return true;
    }
    return false;
}

int DTreeLandPlatformWindowInterface::windowRadius() const
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->windowRadius();
    return 0;
}

void DTreeLandPlatformWindowInterface::setWindowRadius(int windowRadius)
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setWindowRadius(windowRadius);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->windowRadiusChanged();
    }
}

int DTreeLandPlatformWindowInterface::borderWidth() const
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->borderWidth();
    return 0;
}

void DTreeLandPlatformWindowInterface::setBorderWidth(int borderWidth)
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setBorderWidth(borderWidth);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->borderWidthChanged();
    }
}

QColor DTreeLandPlatformWindowInterface::borderColor() const
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->borderColor();
    return {};
}

void DTreeLandPlatformWindowInterface::setBorderColor(const QColor &borderColor)
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setBorderColor(borderColor);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->borderColorChanged();
    }
}

int DTreeLandPlatformWindowInterface::shadowRadius() const
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->shadowRadius();
    return 0;
}

void DTreeLandPlatformWindowInterface::setShadowRadius(int shadowRadius)
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setShadowRadius(shadowRadius);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->shadowRadiusChanged();
    }
}

QPoint DTreeLandPlatformWindowInterface::shadowOffset() const
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->shadowOffset();
    return {};
}

void DTreeLandPlatformWindowInterface::setShadowOffset(const QPoint &shadowOffset)
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setShadowOffset(shadowOffset);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->shadowOffsetChanged();
    }
}

QColor DTreeLandPlatformWindowInterface::shadowColor() const
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->shadowColor();
    return {};
}

void DTreeLandPlatformWindowInterface::setShadowColor(const QColor &shadowColor)
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setShadowColor(shadowColor);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->shadowColorChanged();
    }
}

bool DTreeLandPlatformWindowInterface::enableBlurWindow() const
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->enableBlurWindow();
    return false;
}

void DTreeLandPlatformWindowInterface::setEnableBlurWindow(bool enable)
{
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setEnableBlurWindow(enable);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->enableBlurWindowChanged();
    }
}
DGUI_END_NAMESPACE
