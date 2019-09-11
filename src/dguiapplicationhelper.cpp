/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dguiapplicationhelper.h"
#include "private/dguiapplicationhelper_p.h"
#include "dplatformhandle.h"

#include <QColor>
#include <QPalette>
#include <QWindow>
#include <QGuiApplication>
#include <QPointer>
#include <QPlatformSurfaceEvent>
#include <QDebug>
#include <private/qguiapplication_p.h>

DGUI_BEGIN_NAMESPACE

#define WINDOW_THEME_KEY "_d_platform_theme"

DGuiApplicationHelperPrivate::DGuiApplicationHelperPrivate(DGuiApplicationHelper *qq)
    : DObjectPrivate(qq)
{

}

void DGuiApplicationHelperPrivate::init()
{
    D_Q(DGuiApplicationHelper);

    systemTheme = new DPlatformTheme(0, q);
    // 初始时先将appTheme指定为systtemTheme，在后面合适的地方再初始化appTheme
    appTheme = systemTheme;

    if (qGuiApp) {
        initApplication(qGuiApp);
    } else {
        qAddPreRoutine(staticInitApplication);
    }
}

void DGuiApplicationHelperPrivate::initApplication(QGuiApplication *app)
{
    D_Q(DGuiApplicationHelper);

    q->connect(app, &QGuiApplication::paletteChanged, q, [q, this, app] {
        if (themeType == DGuiApplicationHelper::UnknownType)
            Q_EMIT q->themeTypeChanged(q->toColorType(app->palette()));
    });


    app->connect(systemTheme, &DPlatformTheme::themeNameChanged, app, [this, app] {
        if (appTheme == systemTheme)
            return;

        appTheme->setFallbackProperty(false);
        const QByteArray &theme_name = appTheme->themeName();
        appTheme->setFallbackProperty(true);

        if (!theme_name.isEmpty())
            notifyAppThemeChanged(app);
    });
    app->connect(systemTheme, &DPlatformTheme::activeColorChanged, app, [this, app] {
        if (appTheme == systemTheme)
            return;

        appTheme->setFallbackProperty(false);
        const QColor &active_color = appTheme->activeColor();
        appTheme->setFallbackProperty(true);

        if (!active_color.isValid())
            notifyAppThemeChanged(app);
    });
    app->connect(systemTheme, &DPlatformTheme::paletteChanged, app, [this, app] {
        if (appTheme == systemTheme)
            return;

        if (!appTheme->isValid())
            notifyAppThemeChanged(app);
    });

    if (QGuiApplicationPrivate::is_app_running) {
        _q_initApplicationTheme();
    } else {
        // 延后初始化数据，因为在调用 clientLeader 前必须要保证QGuiApplication已经完全构造完成
        q->metaObject()->invokeMethod(q, "_q_initApplicationTheme", Qt::QueuedConnection, Q_ARG(bool, true));
    }
}

void DGuiApplicationHelperPrivate::staticInitApplication()
{
    if (DGuiApplicationHelper *helper = DGuiApplicationHelper::instance()) {
        helper->d_func()->initApplication(qGuiApp);
    }
}

DPlatformTheme *DGuiApplicationHelperPrivate::initWindow(QWindow *window) const
{
    DPlatformTheme *theme = new DPlatformTheme(window->winId(), appTheme);
    window->setProperty(WINDOW_THEME_KEY, QVariant::fromValue(theme));
    theme->setParent(window); // 跟随窗口销毁

    auto onWindowThemeChanged = [theme, window] {
        qGuiApp->postEvent(window, new QEvent(QEvent::ThemeChange));
    };

    window->connect(theme, &DPlatformTheme::themeNameChanged, window, onWindowThemeChanged);
    window->connect(theme, &DPlatformTheme::activeColorChanged, window, onWindowThemeChanged);
    window->connect(theme, &DPlatformTheme::paletteChanged, window, onWindowThemeChanged);

    return theme;
}

void DGuiApplicationHelperPrivate::_q_initApplicationTheme(bool notifyChange)
{
    if (appTheme && appTheme != systemTheme)
        return;

    appTheme = new DPlatformTheme(DPlatformHandle::windowLeader(), systemTheme);
    QGuiApplication *app = qGuiApp;
    auto onAppThemeChanged = std::bind(&DGuiApplicationHelperPrivate::notifyAppThemeChanged, this, app, false);
    QObject::connect(appTheme, &DPlatformTheme::themeNameChanged, app, onAppThemeChanged);
    QObject::connect(appTheme, &DPlatformTheme::activeColorChanged, app, onAppThemeChanged);
    QObject::connect(appTheme, &DPlatformTheme::paletteChanged, app, onAppThemeChanged);

    if (notifyChange && appTheme->isValid()) {
        notifyAppThemeChanged(app);
    }
}

void DGuiApplicationHelperPrivate::notifyAppThemeChanged(QGuiApplication *app, bool ignorePaletteType)
{
    D_Q(DGuiApplicationHelper);

    if (app->testAttribute(Qt::AA_SetPalette)
            || (!ignorePaletteType && paletteType != DGuiApplicationHelper::UnknownType)) {
        return;
    }

    QWindowSystemInterfacePrivate::ThemeChangeEvent event(nullptr);
    QGuiApplicationPrivate::processThemeChanged(&event);
    Q_EMIT q->themeTypeChanged(q->toColorType(app->palette()));
}

class _DGuiApplicationHelper
{
public:
    _DGuiApplicationHelper()
        : helper(creator())
    {
        helper->initialize();
    }

    ~_DGuiApplicationHelper()
    {
        delete helper;
    }

    static DGuiApplicationHelper *defaultCreator()
    {
        return new DGuiApplicationHelper();
    }

    DGuiApplicationHelper *helper;
    static DGuiApplicationHelper::HelperCreator creator;
};

DGuiApplicationHelper::HelperCreator _DGuiApplicationHelper::creator = _DGuiApplicationHelper::defaultCreator;
Q_GLOBAL_STATIC(_DGuiApplicationHelper, _globalHelper)

DGuiApplicationHelper::DGuiApplicationHelper()
    : QObject(nullptr)
    , DObject(*new DGuiApplicationHelperPrivate(this))
{

}

void DGuiApplicationHelper::initialize()
{
    D_D(DGuiApplicationHelper);

    d->init();
}

void DGuiApplicationHelper::registerInstanceCreator(DGuiApplicationHelper::HelperCreator creator)
{
    _DGuiApplicationHelper::creator = creator;

    if (_globalHelper.exists() && _globalHelper->helper) {
        delete _globalHelper->helper;
        _globalHelper->helper = creator();
        _globalHelper->helper->initialize();
    }
}

inline static int adjustColorValue(int base, qint8 increment, int max = 255)
{
    return increment > 0 ? (max - base) * increment / 100.0 + base
           : base * (1 + increment / 100.0);
}

DGuiApplicationHelper *DGuiApplicationHelper::instance()
{
    return _globalHelper->helper;
}

DGuiApplicationHelper::~DGuiApplicationHelper()
{
    D_D(DGuiApplicationHelper);

    if (d->appPalette) {
        delete d->appPalette;
    }
}

QColor DGuiApplicationHelper::adjustColor(const QColor &base,
                                          qint8 hueFloat, qint8 saturationFloat, qint8 lightnessFloat,
                                          qint8 redFloat, qint8 greenFloat, qint8 blueFloat, qint8 alphaFloat)
{
    // 按HSL格式调整
    int H, S, L, A;
    base.getHsl(&H, &S, &L, &A);

    H = H > 0 ? adjustColorValue(H, hueFloat, 359) : H;
    S = adjustColorValue(S, saturationFloat);
    L = adjustColorValue(L, lightnessFloat);
    A = adjustColorValue(A, alphaFloat);

    QColor new_color = QColor::fromHsl(H, S, L, A);

    // 按RGB格式调整
    int R, G, B;
    new_color.getRgb(&R, &G, &B);

    R = adjustColorValue(R, redFloat);
    G = adjustColorValue(G, greenFloat);
    B = adjustColorValue(B, blueFloat);

    new_color.setRgb(R, G, B, A);

    return new_color;
}

QColor DGuiApplicationHelper::blendColor(const QColor &substrate, const QColor &superstratum)
{
    QColor c2 = superstratum.toRgb();

    if (c2.alpha() >= 255)
        return c2;

    QColor c1 = substrate.toRgb();
    qreal c1_weight = 1 - c2.alphaF();

    int r = c1_weight * c1.red() + c2.alphaF() * c2.red();
    int g = c1_weight * c1.green() + c2.alphaF() * c2.green();
    int b = c1_weight * c1.blue() + c2.alphaF() * c2.blue();

    return QColor(r, g, b, c1.alpha());
}

static QColor light_qpalette[QPalette::NColorRoles] {
    QColor("#414d68"),                  //WindowText
    QColor("#e5e5e5"),                  //Button
    QColor("#e6e6e6"),                  //Light
    QColor("#e5e5e5"),                  //Midlight
    QColor("#e3e3e3"),                  //Dark
    QColor("#e4e4e4"),                  //Mid
    QColor("#414d68"),                  //Text
    QColor("#3768ff"),                  //BrightText
    QColor("#414d68"),                  //ButtonText
    Qt::white,                          //Base
    QColor("#f8f8f8"),                  //Window
    QColor(0, 0, 0, 0.4 * 255),         //Shadow
    QColor("#0081ff"),                  //Highlight
    Qt::white,                          //HighlightedText
    QColor("#0082fa"),                  //Link
    QColor("#ad4579"),                  //LinkVisited
    QColor(0, 0, 0, 0.03 * 255),        //AlternateBase
    Qt::white,                          //NoRole
    QColor(255, 255, 255, 0.8 * 255),   //ToolTipBase
    Qt::black                           //ToolTipText
};

static QColor dark_qpalette[QPalette::NColorRoles] {
    QColor("#c0c6d4"),                  //WindowText
    QColor("#444444"),                  //Button
    QColor("#484848"),                  //Light
    QColor("#474747"),                  //Midlight
    QColor("#414141"),                  //Dark
    QColor("#434343"),                  //Mid
    QColor("#c0c6d4"),                  //Text
    QColor("#3768ff"),                  //BrightText
    QColor("#c0c6d4"),                  //ButtonText
    QColor("#454545"),                  //Base
    QColor("#252525"),                  //Window
    QColor(0, 0, 0, 0.05 * 255),        //Shadow
    QColor("#0081ff"),                  //Highlight
    QColor("#b8d3ff"),                  //HighlightedText
    QColor("#0082fa"),                  //Link
    QColor("#ad4579"),                  //LinkVisited
    QColor(0, 0, 0, 0.05 * 255),        //AlternateBase
    Qt::black,                          //NoRole
    QColor(45, 45, 45, 0.8 * 255),      //ToolTipBase
    QColor("#c0c6d4")                   //ToolTipText
};

static QColor light_dpalette[DPalette::NColorTypes] {
    QColor(),                       //NoType
    QColor(0, 0, 0, 255 * 0.03),    //ItemBackground
    QColor("#001A2E"),              //TextTitle
    QColor("#526A7F"),              //TextTips
    QColor("#FF5736"),              //TextWarning
    QColor("#0082FA"),              //TextLively
    QColor("#25b7ff"),              //LightLively
    QColor("#0098ff"),              //DarkLively
    QColor(0, 0, 0, 0.03 * 255)     //FrameBorder
};

static QColor dark_dpalette[DPalette::NColorTypes] {
    QColor(),                           //NoType
    QColor(255, 255, 255, 255 * 0.05),  //ItemBackground
    QColor("#C0C6D4"),                  //TextTitle
    QColor("#6D7C88"),                  //TextTips
    QColor("#FF5736"),                  //TextWarning
    QColor("#0082FA"),                  //TextLively
    QColor("#0056c1"),                  //LightLively
    QColor("#004c9c"),                  //DarkLively
    QColor(0, 0, 0, 0.05 * 255)         //FrameBorder
};

DPalette DGuiApplicationHelper::standardPalette(DGuiApplicationHelper::ColorType type)
{
    static const DPalette *light_palette = nullptr, *dark_palette = nullptr;

    if (type == LightType) {
        if (Q_LIKELY(light_palette)) {
            return *light_palette;
        }
    } else if (type == DarkType) {
        if (Q_LIKELY(dark_palette)) {
            return *dark_palette;
        }
    } else {
        return DPalette();
    }

    DPalette *pa;
    const QColor *qcolor_list, *dcolor_list;

    if (type == DarkType) {
        pa = new DPalette();
        dark_palette = pa;
        qcolor_list = dark_qpalette;
        dcolor_list = dark_dpalette;
    } else {
        pa = new DPalette();
        light_palette = pa;
        qcolor_list = light_qpalette;
        dcolor_list = light_dpalette;
    }

    for (int i = 0; i < DPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);

        pa->setColor(DPalette::Active, role, qcolor_list[i]);
        generatePaletteColor(*pa, role, type);
    }

    for (int i = 0; i < DPalette::NColorTypes; ++i) {
        DPalette::ColorType role = static_cast<DPalette::ColorType>(i);

        pa->setColor(DPalette::Active, role, dcolor_list[i]);
        generatePaletteColor(*pa, role, type);
    }

    return *const_cast<const DPalette*>(pa);
}

template<typename M>
static void generatePaletteColor_helper(DPalette &base, M role, DGuiApplicationHelper::ColorType type)
{
    if (type == DGuiApplicationHelper::UnknownType) {
        type = DGuiApplicationHelper::toColorType(base);
    }

    QColor disable_mask_color, inactive_mask_color;

    if (type == DGuiApplicationHelper::DarkType) {
        disable_mask_color.setRgba(qRgba(255, 255, 255, 255 * 0.6));
        inactive_mask_color.setRgba(qRgba(255, 255, 255, 255 * 0.4));
    } else {
        disable_mask_color.setRgba(qRgba(255, 255, 255, 255 * 0.8));
        inactive_mask_color.setRgba(qRgba(255, 255, 255, 255 * 0.6));
    }

    const QColor &color = base.color(QPalette::Normal, role);
    base.setColor(QPalette::Disabled, role, DGuiApplicationHelper::blendColor(color, disable_mask_color));
    base.setColor(QPalette::Inactive, role, DGuiApplicationHelper::blendColor(color, inactive_mask_color));
}

void DGuiApplicationHelper::generatePaletteColor(DPalette &base, QPalette::ColorRole role, DGuiApplicationHelper::ColorType type)
{
    if (role == QPalette::Window) {
        const QBrush &window = base.brush(QPalette::Normal, role);
        base.setBrush(QPalette::Disabled, role, window);
        base.setBrush(QPalette::Inactive, role, window);
    }

    generatePaletteColor_helper(base, role, type);
}

void DGuiApplicationHelper::generatePaletteColor(DPalette &base, DPalette::ColorType role, DGuiApplicationHelper::ColorType type)
{
    generatePaletteColor_helper(base, role, type);
}

void DGuiApplicationHelper::generatePalette(DPalette &base, ColorType type)
{
    // 先判断调色板的色调
    if (type == UnknownType) {
        type = toColorType(base);
    }

    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);
        generatePaletteColor(base, role, type);
    }

    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        DPalette::ColorType role = static_cast<DPalette::ColorType>(i);
        generatePaletteColor(base, role, type);
    }
}

DPalette DGuiApplicationHelper::fetchPalette(const DPlatformTheme *theme)
{
    DPalette base_palette;
    const QByteArray theme_name = theme->themeName();
    ColorType type = LightType;

    // 深色主题
    if (theme_name.endsWith("dark")) {
        type = DarkType;
    }

    bool ok = false;
    base_palette = theme->fetchPalette(standardPalette(type), &ok);
    const QColor &active_color = theme->activeColor();

    if (active_color.isValid()) {
        base_palette.setColor(QPalette::Normal, QPalette::Highlight, active_color);

        // ok为true时会整体处理palette的所有颜色
        if (!ok) {
            generatePaletteColor(base_palette, QPalette::Highlight, type);
        }
    }

    if (ok) {
        generatePalette(base_palette, type);
    }

    return base_palette;
}

DPlatformTheme *DGuiApplicationHelper::systemTheme() const
{
    D_DC(DGuiApplicationHelper);

    return d->systemTheme;
}

DPlatformTheme *DGuiApplicationHelper::applicationTheme() const
{
    D_DC(DGuiApplicationHelper);

    return d->appTheme;
}

DPlatformTheme *DGuiApplicationHelper::windowTheme(QWindow *window) const
{
    DPlatformTheme *theme = qvariant_cast<DPlatformTheme*>(window->property(WINDOW_THEME_KEY));

    if (theme) {
        return theme;
    }

    D_DC(DGuiApplicationHelper);
    theme = d->initWindow(window);

    return theme;
}

DPalette DGuiApplicationHelper::applicationPalette() const
{
    D_DC(DGuiApplicationHelper);

    if (d->appPalette) {
        return *d->appPalette;
    }

    ColorType type = UnknownType;

    // 如果应用程序自己设置过palette，则以这个palette为基础获取DPalette
    if (qGuiApp && qGuiApp->testAttribute(Qt::AA_SetPalette)) {
        type = toColorType(qGuiApp->palette());
    } else {
        type = d->paletteType;
    }

    // 如果自定义了palette的类型，则直接返回对应的标准DPalette
    if (type != UnknownType) {
        return standardPalette(type);
    }

    return fetchPalette(d->appTheme);
}

void DGuiApplicationHelper::setApplicationPalette(const DPalette &palette)
{
    D_D(DGuiApplicationHelper);

    if (d->appPalette) {
        if (palette.resolve()) {
            *d->appPalette = palette;
        } else {
            delete d->appPalette;
        }
    } else if (palette.resolve()) {
        d->appPalette = new DPalette(palette);
    } else {
        return;
    }

    d->notifyAppThemeChanged(qGuiApp, true);
}

DPalette DGuiApplicationHelper::windowPalette(QWindow *window) const
{
    D_DC(DGuiApplicationHelper);

    DPlatformTheme *theme = windowTheme(window);

    if (!theme) {
        return applicationPalette();
    }

    return fetchPalette(theme);
}

/*!
 * \~chinese \brief DGuiApplicationHelper::toColorType 获取颜色的明亮度，将其转换为主题类型的枚举值。
 * \~chinese 转换的策略为：先将颜色转换为rgb格式，再根据 Y = 0.299R + 0.587G + 0.114B 的公式
 * \~chinese 计算出颜色的亮度，亮度大于 191 时认为其为浅色，否则认为其为深色。
 * \~chinese \param color 需要转换为主题的类型的颜色
 * \~chinese \return 主题类型的枚举值
 */
DGuiApplicationHelper::ColorType DGuiApplicationHelper::toColorType(const QColor &color)
{
    if (!color.isValid())
        return UnknownType;

    QColor rgb_color = color.toRgb();
    // 获取rgb颜色的亮度(转换为YUV格式)
    float luminance = 0.299 * rgb_color.redF() + 0.587 * rgb_color.greenF() + 0.114 * rgb_color.blueF();

    if (qRound(luminance * 255) > 191) {
        return LightType;
    }

    return DarkType;
}

DGuiApplicationHelper::ColorType DGuiApplicationHelper::toColorType(const QPalette &palette)
{
    return toColorType(palette.background().color());
}

DGuiApplicationHelper::ColorType DGuiApplicationHelper::themeType() const
{
    D_DC(DGuiApplicationHelper);

    if (d->themeType != UnknownType) {
        return d->themeType;
    }

    return qGuiApp ? toColorType(qGuiApp->palette()) : d->themeType;
}

DGuiApplicationHelper::ColorType DGuiApplicationHelper::paletteType() const
{
    D_DC(DGuiApplicationHelper);

    return d->paletteType;
}

void DGuiApplicationHelper::setThemeType(DGuiApplicationHelper::ColorType themeType)
{
    D_D(DGuiApplicationHelper);

    if (d->themeType == themeType)
        return;

    d->themeType = themeType;
    Q_EMIT themeTypeChanged(themeType);
}

void DGuiApplicationHelper::setPaletteType(DGuiApplicationHelper::ColorType paletteType)
{
    D_D(DGuiApplicationHelper);

    if (d->paletteType == paletteType)
        return;

    d->paletteType = paletteType;
    d->notifyAppThemeChanged(qGuiApp, true);

    Q_EMIT paletteTypeChanged(paletteType);
}

DGUI_END_NAMESPACE

#include "moc_dguiapplicationhelper.cpp"
