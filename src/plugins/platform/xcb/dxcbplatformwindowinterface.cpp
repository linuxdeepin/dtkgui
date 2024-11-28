// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dguiapplicationhelper.h"
#include "dxcbplatformwindowinterface.h"
#include "dplatformhandle.h"
#include "dplatformwindowinterface_p_p.h"
#include "dxcbplatformwindowinterface_p.h"
#include "dplatformtheme.h"
#include "dwindowmanagerhelper.h"
#include <dobject.h>
#include <qobject.h>

#include <private/qwaylandwindow_p.h>
#include <dtkcore_global.h>

#include <QGuiApplication>
#include <QDebug>
#include <QPlatformSurfaceEvent>
#include <QStyleHints>
#include <QHash>

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

DXCBPlatformWindowInterfacePrivate::DXCBPlatformWindowInterfacePrivate(DXCBPlatformWindowInterface *qq)
    : DPlatformWindowInterfacePrivate(qq)
{
}

DXCBPlatformWindowInterface::DXCBPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent)
    : DPlatformWindowInterface(*new DXCBPlatformWindowInterfacePrivate(this), window, platformHandle, parent)
{
    D_D(DXCBPlatformWindowInterface);

    /*enableInterfaceForWindow(window);*/

    window->installEventFilter(this);
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

void DXCBPlatformWindowInterface::enableDXcb()
{
    D_D(DXCBPlatformWindowInterface);

    // 优先使用窗口管理器中实现的no titlebar接口实现自定义窗口修饰器的效果
    if (setEnabledNoTitlebar(true)) {
        return;
    }

    if (!isDXcbPlatform())
        return;

    QFunctionPointer enable_dxcb = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    enable_dxcb = qApp->platformFunction(_enableDxcb);
#endif

    if (enable_dxcb) {
        (*reinterpret_cast<bool(*)(QWindow*)>(enable_dxcb))(d->m_window);
    } else if (d->m_window->handle()) {
        Q_ASSERT_X(d->m_window->property(_useDxcb).toBool(), "DXCBPlatformWindowInterfacer:",
                   "Must be called before window handle has been created. See also QWindow::handle()");
    } else {
        d->m_window->setProperty(_useDxcb, true);
    }
}

void DXCBPlatformWindowInterface::enableDXcb(bool redirectContent)
{
    D_D(DXCBPlatformWindowInterface);

    d->m_window->setProperty(_redirectContent, redirectContent);

    enableDXcb();
}

bool DXCBPlatformWindowInterface::isEnabledDXcb()
{
    D_D(DXCBPlatformWindowInterface);

    if (isEnabledNoTitlebar())
        return true;

    QFunctionPointer is_enable_dxcb = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    is_enable_dxcb = qApp->platformFunction(_isEnableDxcb);
#endif

    if (is_enable_dxcb) {
        return (*reinterpret_cast<bool(*)(const QWindow*)>(is_enable_dxcb))(d->m_window);
    }

    return d->m_window->property(_useDxcb).toBool();
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
    D_DC(DXCBPlatformWindowInterface);
    QFunctionPointer is_enable_no_titlebar = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    is_enable_no_titlebar = qApp->platformFunction(_isEnableNoTitlebar);
#endif

    if (is_enable_no_titlebar) {
        return (*reinterpret_cast<bool(*)(const QWindow*)>(is_enable_no_titlebar))(d->m_window);
    }

    return false;
}

bool DXCBPlatformWindowInterface::setEnabledNoTitlebar(bool enable)
{
    D_D(DXCBPlatformWindowInterface);
    auto isDWaylandPlatform = [] {
        return qApp->platformName() == "dwayland" || qApp->property("_d_isDwayland").toBool();
    };
    if (!(isDXcbPlatform() || isDWaylandPlatform() || DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsWaylandPlatform)))
        return false;

    if (isEnabledNoTitlebar() == enable)
        return true;

    QFunctionPointer enable_no_titlear = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    enable_no_titlear = qApp->platformFunction(_setEnableNoTitlebar);
#endif

    if (enable_no_titlear) {
        bool ok = (*reinterpret_cast<bool(*)(QWindow*, bool)>(enable_no_titlear))(d->m_window, enable);
        if (ok && enable) {
            if (d->m_window->handle()) {
                initWindowRadius(d->m_window);
            } else {
                d->m_window->installEventFilter(new CreatorWindowEventFilter(d->m_window));
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
    D_D(DXCBPlatformWindowInterface);

    if (!d->m_window) {
        return false;
    }

    if (isEnabledDXcb()) {
        QVector<quint32> areas;
        for (auto item : area)
            areas << item.x << item.y << item.width << item.height << item.xRadius << item.yRaduis;
        setWindowProperty(d->m_window, _windowBlurAreas, QVariant::fromValue(areas));
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

    QSurfaceFormat format = d->m_window->format();

    format.setAlphaBufferSize(8);
    d->m_window->setFormat(format);

    const qreal device_ratio = d->m_window->devicePixelRatio();

    if (qFuzzyCompare(device_ratio, 1.0)) {
        return reinterpret_cast<bool(*)(const quint32, const QVector<DPlatformHandle::WMBlurArea>&)>(setWmBlurWindowBackgroundArea)(d->m_window->winId(), area);
    }

    QVector<DPlatformHandle::WMBlurArea> new_areas;

    new_areas.reserve(area.size());

    for (const DPlatformHandle::WMBlurArea &a : area) {
        new_areas.append(a * device_ratio);
    }

    return reinterpret_cast<bool(*)(const quint32, const QVector<DPlatformHandle::WMBlurArea>&)>(setWmBlurWindowBackgroundArea)(d->m_window->winId(), new_areas);
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
    D_D(DXCBPlatformWindowInterface);

    if (!d->m_window) {
        return false;
    }

    if (isEnabledDXcb()) {
        setWindowProperty(d->m_window, _windowBlurPaths, QVariant::fromValue(paths));

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

    QSurfaceFormat format = d->m_window->format();

    format.setAlphaBufferSize(8);
    d->m_window->setFormat(format);

    const qreal device_ratio = d->m_window->devicePixelRatio();

    if (qFuzzyCompare(device_ratio, 1.0)) {
        return reinterpret_cast<bool(*)(const quint32, const QList<QPainterPath>&)>(setWmBlurWindowBackgroundPathList)(d->m_window->winId(), paths);
    }

    QList<QPainterPath> new_paths;

    new_paths.reserve(paths.size());

    for (const QPainterPath &p : paths) {
        new_paths.append(p * device_ratio);
    }

    return reinterpret_cast<bool(*)(const quint32, const QList<QPainterPath>&)>(setWmBlurWindowBackgroundPathList)(d->m_window->winId(), new_paths);
}

bool DXCBPlatformWindowInterface::setWindowWallpaperPara(const QRect &area, DPlatformHandle::WallpaperScaleMode sMode, DPlatformHandle::WallpaperFillMode fMode)
{
    D_D(DXCBPlatformWindowInterface);

    if (!d->m_window) {
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

    QSurfaceFormat format = d->m_window->format();

    format.setAlphaBufferSize(8);
    d->m_window->setFormat(format);

    quint32 bMode = sMode | fMode;

    // 激活 backing store
    d->m_window->setProperty("_d_dxcb_wallpaper", QVariant::fromValue(QPair<QRect, int>(area, bMode)));

    if (!d->m_window->handle())  {
        return true;
    } else {
        qWarning() << "because the window handle has been created, so 2D mode will have no effect";
    }

    const qreal device_ratio = d->m_window->devicePixelRatio();
    if (qFuzzyCompare(device_ratio, 1.0) || !area.isValid()) {
        return reinterpret_cast<bool(*)(const quint32, const QRect&, const quint32)>(setWmWallpaperParameter)(d->m_window->winId(), area, bMode);
    }

    QRect new_area(area.x() * device_ratio,
                   area.y() * device_ratio,
                   area.width() * device_ratio,
                   area.height() * device_ratio);

    return reinterpret_cast<bool(*)(const quint32, const QRect&, const quint32)>(setWmWallpaperParameter)(d->m_window->winId(), new_area, bMode);
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

bool DXCBPlatformWindowInterface::setWindowBlurAreaByWM(const QVector<DPlatformHandle::WMBlurArea> &area)
{
    D_D(DXCBPlatformWindowInterface);

    return setWindowBlurArea(area);
}

bool DXCBPlatformWindowInterface::setWindowBlurAreaByWM(const QList<QPainterPath> &paths)
{
    D_D(DXCBPlatformWindowInterface);

    return setWindowBlurArea(paths);
}

void DXCBPlatformWindowInterface::setDisableWindowOverrideCursor(bool disable)
{
    D_D(DXCBPlatformWindowInterface);

    d->m_window->setProperty(_disableOverrideCursor, disable);
}

int DXCBPlatformWindowInterface::windowRadius() const
{
    D_DC(DXCBPlatformWindowInterface);

    return d->m_window->property(_windowRadius).toInt();
}

void DXCBPlatformWindowInterface::setWindowRadius(int windowRadius)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _windowRadius, windowRadius);
    resolve(d->m_window, PropRole::WindowRadius);
}

int DXCBPlatformWindowInterface::borderWidth() const
{
    D_DC(DXCBPlatformWindowInterface);
    return d->m_window->property(_borderWidth).toInt();
}

void DXCBPlatformWindowInterface::setBorderWidth(int borderWidth)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _borderWidth, borderWidth);
}

QColor DXCBPlatformWindowInterface::borderColor() const
{
    D_DC(DXCBPlatformWindowInterface);
    return qvariant_cast<QColor>(d->m_window->property(_borderColor));
}

void DXCBPlatformWindowInterface::setBorderColor(const QColor &borderColor)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _borderColor, QVariant::fromValue(borderColor));
}

int DXCBPlatformWindowInterface::shadowRadius() const
{
    D_DC(DXCBPlatformWindowInterface);
    return d->m_window->property(_shadowRadius).toInt();
}

void DXCBPlatformWindowInterface::setShadowRadius(int shadowRadius)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _shadowRadius, shadowRadius);
}

QPoint DXCBPlatformWindowInterface::shadowOffset() const
{
    D_DC(DXCBPlatformWindowInterface);
    return d->m_window->property(_shadowOffset).toPoint();
}

void DXCBPlatformWindowInterface::setShadowOffset(const QPoint &shadowOffset)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _shadowOffset, shadowOffset);
}

QColor DXCBPlatformWindowInterface::shadowColor() const
{
    D_DC(DXCBPlatformWindowInterface);
    return qvariant_cast<QColor>(d->m_window->property(_shadowColor));
}

void DXCBPlatformWindowInterface::setShadowColor(const QColor &shadowColor)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _shadowColor, QVariant::fromValue(shadowColor));
}

DPlatformHandle::EffectScene DXCBPlatformWindowInterface::windowEffect()
{
    D_DC(DXCBPlatformWindowInterface);
    return qvariant_cast<DPlatformHandle::EffectScene>(d->m_window->property(_windowEffect));
}

void DXCBPlatformWindowInterface::setWindowEffect(DPlatformHandle::EffectScenes effectScene)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _windowEffect, static_cast<quint32>(effectScene));
}

DPlatformHandle::EffectType DXCBPlatformWindowInterface::windowStartUpEffect()
{
    D_DC(DXCBPlatformWindowInterface);
    return qvariant_cast<DPlatformHandle::EffectType>(d->m_window->property(_windowStartUpEffect));
}

void DXCBPlatformWindowInterface::setWindowStartUpEffect(DPlatformHandle::EffectTypes effectType)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _windowStartUpEffect, static_cast<quint32>(effectType));
}

QPainterPath DXCBPlatformWindowInterface::clipPath() const
{
    D_DC(DXCBPlatformWindowInterface);
    return qvariant_cast<QPainterPath>(d->m_window->property(_clipPath));
}

void DXCBPlatformWindowInterface::setClipPath(const QPainterPath &clipPath)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _clipPath, QVariant::fromValue(clipPath));
}

QRegion DXCBPlatformWindowInterface::frameMask() const
{
    D_DC(DXCBPlatformWindowInterface);
    return qvariant_cast<QRegion>(d->m_window->property(_frameMask));
}

void DXCBPlatformWindowInterface::setFrameMask(const QRegion &frameMask)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _frameMask, QVariant::fromValue(frameMask));
}

QMargins DXCBPlatformWindowInterface::frameMargins() const
{
    D_DC(DXCBPlatformWindowInterface);
    return qvariant_cast<QMargins>(d->m_window->property(_frameMargins));
}

bool DXCBPlatformWindowInterface::translucentBackground() const
{
    D_DC(DXCBPlatformWindowInterface);
    return d->m_window->property(_translucentBackground).toBool();
}

void DXCBPlatformWindowInterface::setTranslucentBackground(bool translucentBackground)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _translucentBackground, translucentBackground);
}

bool DXCBPlatformWindowInterface::enableSystemResize() const
{
    D_DC(DXCBPlatformWindowInterface);
    return d->m_window->property(_enableSystemResize).toBool();
}

void DXCBPlatformWindowInterface::setEnableSystemResize(bool enableSystemResize)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _enableSystemResize, enableSystemResize);
}

bool DXCBPlatformWindowInterface::enableSystemMove() const
{
    D_DC(DXCBPlatformWindowInterface);
    return d->m_window->property(_enableSystemMove).toBool();
}

void DXCBPlatformWindowInterface::setEnableSystemMove(bool enableSystemMove)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _enableSystemMove, enableSystemMove);
}

bool DXCBPlatformWindowInterface::enableBlurWindow() const
{
    D_DC(DXCBPlatformWindowInterface);
    return d->m_window->property(_enableBlurWindow).toBool();
}

void DXCBPlatformWindowInterface::setEnableBlurWindow(bool enableBlurWindow)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _enableBlurWindow, enableBlurWindow);
}

bool DXCBPlatformWindowInterface::autoInputMaskByClipPath() const
{
    D_DC(DXCBPlatformWindowInterface);
    return d->m_window->property(_autoInputMaskByClipPath).toBool();
}

void DXCBPlatformWindowInterface::setAutoInputMaskByClipPath(bool autoInputMaskByClipPath)
{
    D_D(DXCBPlatformWindowInterface);
    setWindowProperty(d->m_window, _autoInputMaskByClipPath, autoInputMaskByClipPath);
}

WId DXCBPlatformWindowInterface::realWindowId() const
{
    D_DC(DXCBPlatformWindowInterface);
    return qvariant_cast<WId>(d->m_window->property("_d_real_content_window"));
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


DGUI_END_NAMESPACE
