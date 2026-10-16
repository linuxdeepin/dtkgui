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

#include "treelanddecorationclientextension.h"
#include "treelandbackgroundeffectclientextension.h"

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

        if (!self->m_pressPoint.isNull()
            && (currentPos - self->m_pressPoint).manhattanLength() < QGuiApplication::styleHints()->startDragDistance()) {
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

    connect(BackgroundEffectManager::instance(), &BackgroundEffectManager::blurCapabilityChanged,
            this, [this] {
        const auto initializedBlur = m_initialized & (Features(BlurWindow) | BlurRegion);
        if (initializedBlur) {
            m_dirty |= initializedBlur;
            scheduleApply();
        }
    });

    if (TreelandDecorationManager::instance()->isSupported()
        && !TreelandDecorationManager::instance()->isActive()) {
        // Activation is queued by QWaylandClientExtension; retry when it completes.
        connect(TreelandDecorationManager::instance(), &TreelandDecorationManager::activeChanged, this, &DTreeLandPlatformWindowHelper::onActiveChanged, Qt::QueuedConnection);
    }

    if (window->handle()) {
        initWaylandWindow();
    }
}

DTreeLandPlatformWindowHelper::~DTreeLandPlatformWindowHelper()
{
    if (m_decorationContext) {
        delete m_decorationContext;
        m_decorationContext = nullptr;
    }
    windowMap.remove(static_cast<QWindow*>(parent()));
    delete m_backgroundEffectSurface;
    m_backgroundEffectSurface = nullptr;
}

bool DTreeLandPlatformWindowHelper::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::Expose && m_dirty) {
        scheduleApply();
    }
    if (event->type() == QEvent::Resize && m_blurWindowEnabled) {
        m_dirty |= BlurWindow;
        scheduleApply();
    }
    if (event->type() == QEvent::PlatformSurface) {
        QPlatformSurfaceEvent *se = static_cast<QPlatformSurfaceEvent*>(event);
        if (se->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
            initWaylandWindow();
            onSurfaceCreated();
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
    if (TreelandDecorationManager::instance()->isActive()) {
        if (window()->handle()) {
            initWaylandWindow();
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
    if (m_decorationContext) {
        delete m_decorationContext;
        m_decorationContext = nullptr;
    }
    delete m_backgroundEffectSurface;
    m_backgroundEffectSurface = nullptr;
}

TreelandDecorationContext *DTreeLandPlatformWindowHelper::decorationContext() const
{
    if (m_decorationContext) {
        return m_decorationContext;
    }

    auto manager = TreelandDecorationManager::instance();
    if (!manager->isSupported() || !manager->isInitialized())
        return nullptr;

    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window()->handle());
    // Qt exposes the window after acknowledging the initial xdg_surface configure,
    // including the xdg-decoration negotiation. Earlier decoration overrides are ignored.
    if (!waylandWindow || !waylandWindow->isExposed())
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

    if (!m_decorationContext) {
        const_cast<DTreeLandPlatformWindowHelper *>(this)->m_decorationContext = new TreelandDecorationContext(manager->get_decoration_context(surface));
    }

    return m_decorationContext;
}

BackgroundEffectSurface *DTreeLandPlatformWindowHelper::backgroundEffectSurface() const
{
    if (m_backgroundEffectSurface)
        return m_backgroundEffectSurface;

    auto manager = BackgroundEffectManager::instance();
    if (!manager->isSupported() || !manager->hasBlurCapability() || !manager->isInitialized())
        return nullptr;

    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window()->handle());
    if (!waylandWindow || !waylandWindow->waylandSurface())
        return nullptr;

    auto surface = waylandWindow->waylandSurface()->object();
    if (!surface)
        return nullptr;

    const_cast<DTreeLandPlatformWindowHelper *>(this)->m_backgroundEffectSurface =
        new BackgroundEffectSurface(manager->get_background_effect(surface));
    return m_backgroundEffectSurface;
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

void DTreeLandPlatformWindowHelper::setWindowEffect(DPlatformHandle::EffectScenes effectScene)
{
    updateFeature(m_effectScene, effectScene, WindowEffect);
}

void DTreeLandPlatformWindowHelper::setPlatformHandle(DPlatformHandle *handle)
{
    m_platformHandle = handle;
}

void DTreeLandPlatformWindowHelper::setEnableBlurWindow(bool enableBlurWindow)
{
    updateFeature(m_blurWindowEnabled, enableBlurWindow, BlurWindow);
}

void DTreeLandPlatformWindowHelper::setWindowBlurAreas(const QVector<DPlatformHandle::WMBlurArea> &areas)
{
    QRegion region;
    for (const auto &area : areas) {
        const QRect rect(area.x, area.y, area.width, area.height);
        if (area.xRadius > 0 || area.yRaduis > 0) {
            QPainterPath path;
            path.addRoundedRect(rect, area.xRadius, area.yRaduis, Qt::AbsoluteSize);
            region += QRegion(path.toFillPolygon().toPolygon());
        } else {
            region += rect;
        }
    }
    updateFeature(m_blurRegion, region, BlurRegion);
}

void DTreeLandPlatformWindowHelper::setWindowBlurPaths(const QList<QPainterPath> &paths)
{
    QRegion region;
    for (const auto &path : paths) {
        region += QRegion(path.toFillPolygon().toPolygon(), path.fillRule());
    }
    updateFeature(m_blurRegion, region, BlurRegion);
}

void DTreeLandPlatformWindowHelper::scheduleApply()
{
    if (m_applyScheduled)
        return;
    m_applyScheduled = true;
    QMetaObject::invokeMethod(this, &DTreeLandPlatformWindowHelper::applyPending, Qt::QueuedConnection);
}

void DTreeLandPlatformWindowHelper::applyPending()
{
    m_applyScheduled = false;

    if (auto context = decorationContext()) {
        if (m_dirty & NoTitlebar) {
            m_dirty &= ~NoTitlebar;
            context->set_titlebar_mode(m_noTitlebar ? TreelandDecorationContext::titlebar_mode_hide : TreelandDecorationContext::titlebar_mode_show);
        }
        if (m_dirty & Radius) {
            m_dirty &= ~Radius;
            context->set_corner_radius(m_radius);
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
        if (m_dirty & WindowEffect) {
            m_dirty &= ~WindowEffect;
            if (m_effectScene.testFlag(DPlatformHandle::EffectNoRadius)) {
                context->set_corner_radius(0);
            }
            if (m_effectScene.testFlag(DPlatformHandle::EffectNoShadow)) {
                context->set_shadow(0, 0, 0, 0, 0, 0, 0);
            }
            if (m_effectScene.testFlag(DPlatformHandle::EffectNoBorder)) {
                context->set_border(0, 0, 0, 0, 0);
            }
        }
    }

    const auto blurFeatures = Features(BlurWindow) | BlurRegion;
    if (m_dirty & blurFeatures) {
        const QRegion region = m_blurWindowEnabled
            ? QRegion(QRect(QPoint(0, 0), window()->size())) : m_blurRegion;
        auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window()->handle());
        bool changed = false;
        if (region.isEmpty()) {
            changed = m_backgroundEffectSurface != nullptr;
            delete m_backgroundEffectSurface;
            m_backgroundEffectSurface = nullptr;
            m_dirty &= ~blurFeatures;
        } else {
            auto manager = BackgroundEffectManager::instance();
            if (manager->hasBlurCapability() && waylandWindow) {
                if (auto effect = backgroundEffectSurface()) {
                    effect->setBlurRegion(region, waylandWindow->display());
                    m_dirty &= ~blurFeatures;
                    changed = true;
                }
            }
        }
        // Unconfigured surfaces receive the pending blur state with their first frame.
        if (changed && waylandWindow && waylandWindow->isExposed())
            waylandWindow->commit();
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
    if (!TreelandDecorationManager::instance()->isSupported())
        return false;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->isEnabledNoTitlebar();
    return false;
}

bool DTreeLandPlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return false;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setEnabledNoTitlebar(enable);
        return true;
    }
    return false;
}

int DTreeLandPlatformWindowInterface::windowRadius() const
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return 0;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->windowRadius();
    return 0;
}

void DTreeLandPlatformWindowInterface::setWindowRadius(int windowRadius)
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setWindowRadius(windowRadius);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->windowRadiusChanged();
    }
}

int DTreeLandPlatformWindowInterface::borderWidth() const
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return 0;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->borderWidth();
    return 0;
}

void DTreeLandPlatformWindowInterface::setBorderWidth(int borderWidth)
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setBorderWidth(borderWidth);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->borderWidthChanged();
    }
}

QColor DTreeLandPlatformWindowInterface::borderColor() const
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return {};
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->borderColor();
    return {};
}

void DTreeLandPlatformWindowInterface::setBorderColor(const QColor &borderColor)
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setBorderColor(borderColor);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->borderColorChanged();
    }
}

int DTreeLandPlatformWindowInterface::shadowRadius() const
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return 0;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->shadowRadius();
    return 0;
}

void DTreeLandPlatformWindowInterface::setShadowRadius(int shadowRadius)
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setShadowRadius(shadowRadius);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->shadowRadiusChanged();
    }
}

QPoint DTreeLandPlatformWindowInterface::shadowOffset() const
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return {};
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->shadowOffset();
    return {};
}

void DTreeLandPlatformWindowInterface::setShadowOffset(const QPoint &shadowOffset)
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setShadowOffset(shadowOffset);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->shadowOffsetChanged();
    }
}

QColor DTreeLandPlatformWindowInterface::shadowColor() const
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return {};
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->shadowColor();
    return {};
}

void DTreeLandPlatformWindowInterface::setShadowColor(const QColor &shadowColor)
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setShadowColor(shadowColor);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->shadowColorChanged();
    }
}

bool DTreeLandPlatformWindowInterface::enableBlurWindow() const
{
    if (!BackgroundEffectManager::instance()->isSupported())
        return false;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return helper->enableBlurWindow();
    return false;
}

void DTreeLandPlatformWindowInterface::setEnableBlurWindow(bool enable)
{
    if (!BackgroundEffectManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setEnableBlurWindow(enable);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->enableBlurWindowChanged();
    }
}

DPlatformHandle::EffectScene DTreeLandPlatformWindowInterface::windowEffect()
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return {};
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window))
        return static_cast<DPlatformHandle::EffectScene>(static_cast<int>(helper->windowEffect()));
    return {};
}

void DTreeLandPlatformWindowInterface::setWindowEffect(DPlatformHandle::EffectScenes effectScene)
{
    if (!TreelandDecorationManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setWindowEffect(effectScene);
        if (m_platformHandle)
            Q_EMIT m_platformHandle->windowEffectChanged();
    }
}

void DTreeLandPlatformWindowInterface::setWindowBlurAreas(const QVector<DPlatformHandle::WMBlurArea> &areas)
{
    if (!BackgroundEffectManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setWindowBlurAreas(areas);
    }
}

void DTreeLandPlatformWindowInterface::setWindowBlurPaths(const QList<QPainterPath> &paths)
{
    if (!BackgroundEffectManager::instance()->isSupported())
        return;
    if (auto helper = DTreeLandPlatformWindowHelper::get(m_window)) {
        helper->setWindowBlurPaths(paths);
    }
}

DGUI_END_NAMESPACE
