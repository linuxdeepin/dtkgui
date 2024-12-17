// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dxcbplatformwindowinterface.h"
#include "dguiapplicationhelper.h"
#include "dplatformtheme.h"
#include "dwindowmanagerhelper.h"

#include <private/qwaylandwindow_p.h>

#include <QGuiApplication>
#include <QPlatformSurfaceEvent>
#include <QStyleHints>

DGUI_BEGIN_NAMESPACE

#define DXCB_PLUGIN_KEY "dxcb"
#define DXCB_PLUGIN_SYMBOLIC_PROPERTY "_d_isDxcb"

#define DEFINE_CONST_CHAR(Name) const char _##Name[] = "_d_" #Name

DEFINE_CONST_CHAR(useDxcb);
DEFINE_CONST_CHAR(redirectContent);
DEFINE_CONST_CHAR(netWmStates);
DEFINE_CONST_CHAR(windowRadius);
DEFINE_CONST_CHAR(borderWidth);
DEFINE_CONST_CHAR(borderColor);
DEFINE_CONST_CHAR(windowEffect);
DEFINE_CONST_CHAR(windowStartUpEffect);
DEFINE_CONST_CHAR(shadowRadius);
DEFINE_CONST_CHAR(shadowOffset);
DEFINE_CONST_CHAR(shadowColor);
DEFINE_CONST_CHAR(clipPath);
DEFINE_CONST_CHAR(frameMask);
DEFINE_CONST_CHAR(frameMargins);
DEFINE_CONST_CHAR(translucentBackground);
DEFINE_CONST_CHAR(enableSystemResize);
DEFINE_CONST_CHAR(enableSystemMove);
DEFINE_CONST_CHAR(enableBlurWindow);
DEFINE_CONST_CHAR(windowBlurAreas);
DEFINE_CONST_CHAR(windowBlurPaths);
DEFINE_CONST_CHAR(windowWallpaperParas);
DEFINE_CONST_CHAR(autoInputMaskByClipPath);

DEFINE_CONST_CHAR(resolve_mask);
enum PropRole {
    WindowRadius,

    // TO BE CONTINUE
};

// functions
DEFINE_CONST_CHAR(setWmBlurWindowBackgroundArea);
DEFINE_CONST_CHAR(setWmBlurWindowBackgroundPathList);
DEFINE_CONST_CHAR(setWmBlurWindowBackgroundMaskImage);
DEFINE_CONST_CHAR(setWmWallpaperParameter);
DEFINE_CONST_CHAR(setWindowProperty);
DEFINE_CONST_CHAR(pluginVersion);
DEFINE_CONST_CHAR(disableOverrideCursor);
DEFINE_CONST_CHAR(enableDxcb);
DEFINE_CONST_CHAR(isEnableDxcb);
DEFINE_CONST_CHAR(setEnableNoTitlebar);
DEFINE_CONST_CHAR(isEnableNoTitlebar);
DEFINE_CONST_CHAR(clientLeader);

static void resolve(QObject *obj, PropRole role)
{
    int mask = obj->property(_resolve_mask).toInt();
    obj->setProperty(_resolve_mask, (mask |= 1 << role));
}

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

DXCBPlatformWindowInterface::DXCBPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent)
    : QObject(parent)
    , DPlatformWindowInterface(window, platformHandle)
{
    if (window) {
        window->installEventFilter(this);
    }
}

DXCBPlatformWindowInterface::~DXCBPlatformWindowInterface()
{
}

QString DXCBPlatformWindowInterface::pluginVersion()
{
    QFunctionPointer pv = 0;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    pv = qApp->platformFunction(_pluginVersion);
#endif

    if (Q_UNLIKELY(!pv))
        return QString();

    return reinterpret_cast<QString(*)()>(pv)();
}

bool DXCBPlatformWindowInterface::isDXcbPlatform()
{
    if (!qApp)
        return false;

    static bool _is_dxcb = qApp->platformName() == DXCB_PLUGIN_KEY || qApp->property(DXCB_PLUGIN_SYMBOLIC_PROPERTY).toBool();

    return _is_dxcb;
}

bool DXCBPlatformWindowInterface::connectWindowManagerChangedSignal(QObject *object, std::function<void ()> slot)
{
    if (object) {
        return QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::windowManagerChanged, object, slot);
    }

    return QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::windowManagerChanged, slot);
}

bool DXCBPlatformWindowInterface::connectHasBlurWindowChanged(QObject *object, std::function<void ()> slot)
{
    if (object) {
        return QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasBlurWindowChanged, object, slot);
    }

    return QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasBlurWindowChanged, slot);
}

WId DXCBPlatformWindowInterface::windowLeader()
{
    QFunctionPointer clientLeader = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    clientLeader = qApp->platformFunction(_clientLeader);
#endif

    if (!clientLeader) {
        return 0;
    }

    return reinterpret_cast<quint32(*)()>(clientLeader)();
}

void DXCBPlatformWindowInterface::setEnabled(bool enabled)
{
    // 优先使用窗口管理器中实现的no titlebar接口实现自定义窗口修饰器的效果
    if (setEnabledNoTitlebar(enabled)) {
        return;
    }

    if (!isDXcbPlatform())
        return;

    QFunctionPointer enable_dxcb = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    enable_dxcb = qApp->platformFunction(_enableDxcb);
#endif

    if (enable_dxcb) {
        (*reinterpret_cast<bool(*)(QWindow*)>(enable_dxcb))(m_window);
    } else if (m_window->handle()) {
        Q_ASSERT_X(m_window->property(_useDxcb).toBool(), "DXCBPlatformWindowInterfacer:",
                   "Must be called before window handle has been created. See also QWindow::handle()");
    } else {
        m_window->setProperty(_useDxcb, enabled);
    }
}

void DXCBPlatformWindowInterface::enableDXcb(bool redirectContent)
{
    m_window->setProperty(_redirectContent, redirectContent);

    setEnabled(true);
}

bool DXCBPlatformWindowInterface::isEnabled() const
{
    if (isEnabledNoTitlebar())
        return true;

    QFunctionPointer is_enable_dxcb = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    is_enable_dxcb = qApp->platformFunction(_isEnableDxcb);
#endif

    if (is_enable_dxcb) {
        return (*reinterpret_cast<bool(*)(const QWindow*)>(is_enable_dxcb))(m_window);
    }

    return m_window->property(_useDxcb).toBool();
}

bool DXCBPlatformWindowInterface::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_window && m_platformHandle) {
        if (event->type() == QEvent::DynamicPropertyChange) {
            QDynamicPropertyChangeEvent *e = static_cast<QDynamicPropertyChangeEvent *>(event);

            if (e->propertyName() == _windowRadius) {
                Q_EMIT m_platformHandle->windowRadiusChanged();
            } else if (e->propertyName() == _borderWidth) {
                Q_EMIT m_platformHandle->borderWidthChanged();
            } else if (e->propertyName() == _borderColor) {
                Q_EMIT m_platformHandle->borderColorChanged();
            } else if (e->propertyName() == _shadowRadius) {
                Q_EMIT m_platformHandle->shadowRadiusChanged();
            } else if (e->propertyName() == _shadowOffset) {
                Q_EMIT m_platformHandle->shadowOffsetChanged();
            } else if (e->propertyName() == _shadowColor) {
                Q_EMIT m_platformHandle->shadowColorChanged();
            } else if (e->propertyName() == _clipPath) {
                Q_EMIT m_platformHandle->clipPathChanged();
            } else if (e->propertyName() == _frameMask) {
                Q_EMIT m_platformHandle->frameMaskChanged();
            } else if (e->propertyName() == _frameMargins) {
                Q_EMIT m_platformHandle->frameMarginsChanged();
            } else if (e->propertyName() == _translucentBackground) {
                Q_EMIT m_platformHandle->translucentBackgroundChanged();
            } else if (e->propertyName() == _enableSystemResize) {
                Q_EMIT m_platformHandle->enableSystemResizeChanged();
            } else if (e->propertyName() == _enableSystemMove) {
                Q_EMIT m_platformHandle->enableSystemMoveChanged();
            } else if (e->propertyName() == _enableBlurWindow) {
                Q_EMIT m_platformHandle->enableBlurWindowChanged();
            } else if (e->propertyName() == _autoInputMaskByClipPath) {
                Q_EMIT m_platformHandle->autoInputMaskByClipPathChanged();
            }
        }
    }

    return QObject::eventFilter(obj, event);
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

class Q_DECL_HIDDEN CreatorWindowEventFilter : public QObject {
public:
    CreatorWindowEventFilter(QObject *par= nullptr): QObject(par){}

public:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::PlatformSurface) {
            QPlatformSurfaceEvent *se = static_cast<QPlatformSurfaceEvent*>(event);
            if (se->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {  // 若收到此信号， 则 WinID 已被创建
                auto window = qobject_cast<QWindow *>(watched);
                initWindowRadius(window);
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

bool DXCBPlatformWindowInterface::isEnabledNoTitlebar() const
{
    QFunctionPointer is_enable_no_titlebar = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    is_enable_no_titlebar = qApp->platformFunction(_isEnableNoTitlebar);
#endif

    if (is_enable_no_titlebar) {
        return (*reinterpret_cast<bool(*)(const QWindow*)>(is_enable_no_titlebar))(m_window);
    }

    return false;
}

bool DXCBPlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    if (isEnabledNoTitlebar() == enable)
        return true;

    QFunctionPointer enable_no_titlear = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    enable_no_titlear = qApp->platformFunction(_setEnableNoTitlebar);
#endif

    if (enable_no_titlear) {
        bool ok = (*reinterpret_cast<bool(*)(QWindow*, bool)>(enable_no_titlear))(m_window, enable);
        if (ok && enable) {
            if (m_window->handle()) {
                initWindowRadius(m_window);
            } else {
                m_window->installEventFilter(new CreatorWindowEventFilter(m_window));
            }
        }

        return ok;
    }

    return false;
}

inline DPlatformHandle::WMBlurArea operator *(const DPlatformHandle::WMBlurArea &area, qreal scale)
{
    if (qFuzzyCompare(scale, 1.0))
        return area;

    DPlatformHandle::WMBlurArea new_area;

    new_area.x = qRound64(area.x * scale);
    new_area.y = qRound64(area.y * scale);
    new_area.width = qRound64(area.width * scale);
    new_area.height = qRound64(area.height * scale);
    new_area.xRadius = qRound64(area.xRadius * scale);
    new_area.yRaduis = qRound64(area.yRaduis * scale);

    return new_area;
}

/*!
  \brief DXCBPlatformWindowInterface::setWindowBlurAreaByWM
  设置窗口背景的模糊区域，示例：
  \code
  QWindow w;
  QVector<DXCBPlatformWindowInterface::WMBlurArea> area_list;
  DXCBPlatformWindowInterface::WMBlurArea area;

  area.x = 50;
  area.y = 50;
  area.width = 200;
  area.height = 200;
  area.xRadius = 10;
  area.yRaduis = 10;
  area_list.append(area);

  DXCBPlatformWindowInterface::setWindowBlurAreaByWM(&w, area_list);

  QSurfaceFormat format = w.format();
  format.setAlphaBufferSize(8);

  w.setFormat(format);
  w.resize(300, 300);
  w.show();

  \endcode
  \image blur_window_demo1.png
  \a window 目标窗口对象
  \a area 模糊区域，此区域范围内的窗口背景将填充为窗口后面内容模糊之后的图像
  \return 如果设置成功则返回 true，否则返回 false
  \note 对于需要显示模糊背景的窗口，需要将其 QSurfaceFormat 的 alpha 通道设置为8
  \note 调用此接口设置窗口背景模糊区域后将覆盖之前所设置的区域，包括调用
  setWindowBlurAreaByWM(QWindow *, const QList<QPainterPath> &)
  所设置的区域
  \note 建议使用 DBlurEffectWidget 实现窗口背景模糊效果
  \note 此功能依赖于窗口管理器的实现，目前仅支持 deepin-wm 和 kwin 这两个窗口管理器
  \sa Dtk::Widget::DBlurEffectWidget
  \sa QSurfaceFormat::setAlphaBufferSize
  \sa QWindow::setFormat
  \sa DWindowManagerHelper::hasBlurWindow
  \sa DXCBPlatformWindowInterface::setWindowBlurAreaByWM(QWindow *, const QList<QPainterPath> &)
 */
bool DXCBPlatformWindowInterface::setWindowBlurArea(const QVector<DPlatformHandle::WMBlurArea> &area)
{
    if (!m_window) {
        return false;
    }

    if (isEnabled()) {
        QVector<quint32> areas;
        for (auto item : area)
            areas << item.x << item.y << item.width << item.height << item.xRadius << item.yRaduis;
        setWindowProperty(m_window, _windowBlurAreas, QVariant::fromValue(areas));
        return true;
    }

    QFunctionPointer setWmBlurWindowBackgroundArea = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    setWmBlurWindowBackgroundArea = qApp->platformFunction(_setWmBlurWindowBackgroundArea);
#endif

    if (!setWmBlurWindowBackgroundArea) {
        qWarning("setWindowBlurAreaByWM is not support");

        return false;
    }

    QSurfaceFormat format = m_window->format();

    format.setAlphaBufferSize(8);
    m_window->setFormat(format);

    const qreal device_ratio = m_window->devicePixelRatio();

    if (qFuzzyCompare(device_ratio, 1.0)) {
        return reinterpret_cast<bool(*)(const quint32, const QVector<DPlatformHandle::WMBlurArea>&)>(setWmBlurWindowBackgroundArea)(m_window->winId(), area);
    }

    QVector<DPlatformHandle::WMBlurArea> new_areas;

    new_areas.reserve(area.size());

    for (const DPlatformHandle::WMBlurArea &a : area) {
        new_areas.append(a * device_ratio);
    }

    return reinterpret_cast<bool(*)(const quint32, const QVector<DPlatformHandle::WMBlurArea>&)>(setWmBlurWindowBackgroundArea)(m_window->winId(), new_areas);
}

inline QPainterPath operator *(const QPainterPath &path, qreal scale)
{
    if (qFuzzyCompare(1.0, scale))
        return path;

    QPainterPath new_path = path;

    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);

        new_path.setElementPositionAt(i, qRound(e.x * scale), qRound(e.y * scale));
    }

    return new_path;
}

bool DXCBPlatformWindowInterface::setWindowBlurArea(const QList<QPainterPath> &paths)
{
    if (!m_window) {
        return false;
    }

    if (isEnabled()) {
        setWindowProperty(m_window, _windowBlurPaths, QVariant::fromValue(paths));

        return true;
    }

    QFunctionPointer setWmBlurWindowBackgroundPathList = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    setWmBlurWindowBackgroundPathList = qApp->platformFunction(_setWmBlurWindowBackgroundPathList);
#endif

    if (!setWmBlurWindowBackgroundPathList) {
        qWarning("setWindowBlurAreaByWM is not support");

        return false;
    }

    QSurfaceFormat format = m_window->format();

    format.setAlphaBufferSize(8);
    m_window->setFormat(format);

    const qreal device_ratio = m_window->devicePixelRatio();

    if (qFuzzyCompare(device_ratio, 1.0)) {
        return reinterpret_cast<bool(*)(const quint32, const QList<QPainterPath>&)>(setWmBlurWindowBackgroundPathList)(m_window->winId(), paths);
    }

    QList<QPainterPath> new_paths;

    new_paths.reserve(paths.size());

    for (const QPainterPath &p : paths) {
        new_paths.append(p * device_ratio);
    }

    return reinterpret_cast<bool(*)(const quint32, const QList<QPainterPath>&)>(setWmBlurWindowBackgroundPathList)(m_window->winId(), new_paths);
}

bool DXCBPlatformWindowInterface::setWindowWallpaperPara(const QRect &area, DPlatformHandle::WallpaperScaleMode sMode, DPlatformHandle::WallpaperFillMode fMode)
{
    if (!m_window) {
        return false;
    }

    QFunctionPointer setWmWallpaperParameter = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    setWmWallpaperParameter = qApp->platformFunction(_setWmWallpaperParameter);
#endif

    if (!setWmWallpaperParameter) {
        qWarning("setWindowWallpaperParaByWM is not support");

        return false;
    }

    QSurfaceFormat format = m_window->format();

    format.setAlphaBufferSize(8);
    m_window->setFormat(format);

    quint32 bMode = sMode | fMode;

    // 激活 backing store
    m_window->setProperty("_d_dxcb_wallpaper", QVariant::fromValue(QPair<QRect, int>(area, bMode)));

    if (!m_window->handle())  {
        return true;
    } else {
        qWarning() << "because the window handle has been created, so 2D mode will have no effect";
    }

    const qreal device_ratio = m_window->devicePixelRatio();
    if (qFuzzyCompare(device_ratio, 1.0) || !area.isValid()) {
        return reinterpret_cast<bool(*)(const quint32, const QRect&, const quint32)>(setWmWallpaperParameter)(m_window->winId(), area, bMode);
    }

    QRect new_area(area.x() * device_ratio,
                   area.y() * device_ratio,
                   area.width() * device_ratio,
                   area.height() * device_ratio);

    return reinterpret_cast<bool(*)(const quint32, const QRect&, const quint32)>(setWmWallpaperParameter)(m_window->winId(), new_area, bMode);
}

void DXCBPlatformWindowInterface::setDisableWindowOverrideCursor(bool disable)
{
    m_window->setProperty(_disableOverrideCursor, disable);
}

int DXCBPlatformWindowInterface::windowRadius() const
{
    return m_window->property(_windowRadius).toInt();
}

void DXCBPlatformWindowInterface::setWindowRadius(int windowRadius)
{
    setWindowProperty(m_window, _windowRadius, windowRadius);
    resolve(m_window, PropRole::WindowRadius);
}

int DXCBPlatformWindowInterface::borderWidth() const
{
    return m_window->property(_borderWidth).toInt();
}

void DXCBPlatformWindowInterface::setBorderWidth(int borderWidth)
{
    setWindowProperty(m_window, _borderWidth, borderWidth);
}

QColor DXCBPlatformWindowInterface::borderColor() const
{
    return qvariant_cast<QColor>(m_window->property(_borderColor));
}

void DXCBPlatformWindowInterface::setBorderColor(const QColor &borderColor)
{
    setWindowProperty(m_window, _borderColor, QVariant::fromValue(borderColor));
}

int DXCBPlatformWindowInterface::shadowRadius() const
{
    return m_window->property(_shadowRadius).toInt();
}

void DXCBPlatformWindowInterface::setShadowRadius(int shadowRadius)
{
    setWindowProperty(m_window, _shadowRadius, shadowRadius);
}

QPoint DXCBPlatformWindowInterface::shadowOffset() const
{
    return m_window->property(_shadowOffset).toPoint();
}

void DXCBPlatformWindowInterface::setShadowOffset(const QPoint &shadowOffset)
{
    setWindowProperty(m_window, _shadowOffset, shadowOffset);
}

QColor DXCBPlatformWindowInterface::shadowColor() const
{
    return qvariant_cast<QColor>(m_window->property(_shadowColor));
}

void DXCBPlatformWindowInterface::setShadowColor(const QColor &shadowColor)
{
    setWindowProperty(m_window, _shadowColor, QVariant::fromValue(shadowColor));
}

DPlatformHandle::EffectScene DXCBPlatformWindowInterface::windowEffect()
{
    return qvariant_cast<DPlatformHandle::EffectScene>(m_window->property(_windowEffect));
}

void DXCBPlatformWindowInterface::setWindowEffect(DPlatformHandle::EffectScenes effectScene)
{
    setWindowProperty(m_window, _windowEffect, static_cast<quint32>(effectScene));
}

DPlatformHandle::EffectType DXCBPlatformWindowInterface::windowStartUpEffect()
{
    return qvariant_cast<DPlatformHandle::EffectType>(m_window->property(_windowStartUpEffect));
}

void DXCBPlatformWindowInterface::setWindowStartUpEffect(DPlatformHandle::EffectTypes effectType)
{
    setWindowProperty(m_window, _windowStartUpEffect, static_cast<quint32>(effectType));
}

QPainterPath DXCBPlatformWindowInterface::clipPath() const
{
    return qvariant_cast<QPainterPath>(m_window->property(_clipPath));
}

void DXCBPlatformWindowInterface::setClipPath(const QPainterPath &clipPath)
{
    setWindowProperty(m_window, _clipPath, QVariant::fromValue(clipPath));
}

QRegion DXCBPlatformWindowInterface::frameMask() const
{
    return qvariant_cast<QRegion>(m_window->property(_frameMask));
}

void DXCBPlatformWindowInterface::setFrameMask(const QRegion &frameMask)
{
    setWindowProperty(m_window, _frameMask, QVariant::fromValue(frameMask));
}

QMargins DXCBPlatformWindowInterface::frameMargins() const
{
    return qvariant_cast<QMargins>(m_window->property(_frameMargins));
}

bool DXCBPlatformWindowInterface::translucentBackground() const
{
    return m_window->property(_translucentBackground).toBool();
}

void DXCBPlatformWindowInterface::setTranslucentBackground(bool translucentBackground)
{
    setWindowProperty(m_window, _translucentBackground, translucentBackground);
}

bool DXCBPlatformWindowInterface::enableSystemResize() const
{
    return m_window->property(_enableSystemResize).toBool();
}

void DXCBPlatformWindowInterface::setEnableSystemResize(bool enableSystemResize)
{
    setWindowProperty(m_window, _enableSystemResize, enableSystemResize);
}

bool DXCBPlatformWindowInterface::enableSystemMove() const
{
    return m_window->property(_enableSystemMove).toBool();
}

void DXCBPlatformWindowInterface::setEnableSystemMove(bool enableSystemMove)
{
    setWindowProperty(m_window, _enableSystemMove, enableSystemMove);
}

bool DXCBPlatformWindowInterface::enableBlurWindow() const
{
    return m_window->property(_enableBlurWindow).toBool();
}

void DXCBPlatformWindowInterface::setEnableBlurWindow(bool enableBlurWindow)
{
    setWindowProperty(m_window, _enableBlurWindow, enableBlurWindow);
}

bool DXCBPlatformWindowInterface::autoInputMaskByClipPath() const
{
    return m_window->property(_autoInputMaskByClipPath).toBool();
}

void DXCBPlatformWindowInterface::setAutoInputMaskByClipPath(bool autoInputMaskByClipPath)
{
    setWindowProperty(m_window, _autoInputMaskByClipPath, autoInputMaskByClipPath);
}

WId DXCBPlatformWindowInterface::realWindowId() const
{
    return qvariant_cast<WId>(m_window->property("_d_real_content_window"));
}

DGUI_END_NAMESPACE
