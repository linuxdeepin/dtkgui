/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DGUIAPPLICATIONHELPER_H
#define DGUIAPPLICATIONHELPER_H

#include <dtkgui_global.h>
#include <DPalette>
#include <DObject>

#include <QGuiApplication>
#include <QObject>

DGUI_BEGIN_NAMESPACE

class DPlatformTheme;
class DFontManager;
class DGuiApplicationHelperPrivate;
class DGuiApplicationHelper : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DGuiApplicationHelper)

    Q_PROPERTY(ColorType themeType READ themeType NOTIFY themeTypeChanged)
    Q_PROPERTY(ColorType paletteType READ paletteType WRITE setPaletteType NOTIFY paletteTypeChanged)

public:
    enum ColorType {
        UnknownType,
        LightType,
        DarkType
    };
    Q_ENUM(ColorType)

    enum SingleScope {
        UserScope,
        GroupScope,
        WorldScope
    };
    Q_ENUM(SingleScope)

    enum Attribute {
        UseInactiveColorGroup    = 1 << 0,
        ColorCompositing         = 1 << 1,

        /* readonly flag */
        ReadOnlyLimit            = 1 << 22,
        IsDeepinPlatformTheme    = ReadOnlyLimit << 0,
        IsDXcbPlatform           = ReadOnlyLimit << 1,
        IsXWindowPlatform        = ReadOnlyLimit << 2,
        IsTableEnvironment       = ReadOnlyLimit << 3,
        IsDeepinEnvironment      = ReadOnlyLimit << 4,
        IsSpecialEffectsEnvironment = ReadOnlyLimit << 5
    };
    Q_ENUM(Attribute)
    Q_DECLARE_FLAGS(Attributes, Attribute)

    typedef DGuiApplicationHelper *(*HelperCreator)();
    D_DECL_DEPRECATED static void registerInstanceCreator(HelperCreator creator);
    static DGuiApplicationHelper *instance();
    ~DGuiApplicationHelper();

    static QColor adjustColor(const QColor &base, qint8 hueFloat, qint8 saturationFloat, qint8 lightnessFloat,
                              qint8 redFloat, qint8 greenFloat, qint8 blueFloat, qint8 alphaFloat);
    static QColor blendColor(const QColor &substrate, const QColor &superstratum);
    static DPalette standardPalette(ColorType type);
    static void generatePaletteColor(DPalette &base, QPalette::ColorRole role, ColorType type);
    static void generatePaletteColor(DPalette &base, DPalette::ColorType role, ColorType type);
    static void generatePalette(DPalette &base, ColorType type = UnknownType);
    static DPalette fetchPalette(const DPlatformTheme *theme);
    Q_DECL_DEPRECATED_X("Use UseInactiveColorGroup enum with setAttribute.") static void setUseInactiveColorGroup(bool on);
    Q_DECL_DEPRECATED_X("Use ColorCompositing enum with setAttribute.") static void setColorCompositingEnabled(bool on);
    static bool isXWindowPlatform();
    static bool isTabletEnvironment();
    static bool isSpecialEffectsEnvironment();
    static void setAttribute(Attribute attribute, bool enable);
    static bool testAttribute(Attribute attribute);

    DPlatformTheme *systemTheme() const;
    DPlatformTheme *applicationTheme() const;
    D_DECL_DEPRECATED DPlatformTheme *windowTheme(QWindow *window) const;

    DPalette applicationPalette() const;
    void setApplicationPalette(const DPalette &palette);
    D_DECL_DEPRECATED DPalette windowPalette(QWindow *window) const;

    const DFontManager *fontManager() const;

    static ColorType toColorType(const QColor &color);
    static ColorType toColorType(const QPalette &palette);
    ColorType themeType() const;
    ColorType paletteType() const;

    static bool setSingleInstance(const QString &key, SingleScope singleScope = UserScope);
    static void setSingleInstanceInterval(int interval = 3000);
    D_DECL_DEPRECATED static void setSingelInstanceInterval(int interval = 3000);

public Q_SLOTS:
    D_DECL_DEPRECATED_X("Plase use setPaletteType") void setThemeType(ColorType themeType);
    void setPaletteType(ColorType paletteType);

Q_SIGNALS:
    void themeTypeChanged(ColorType themeType);
    void paletteTypeChanged(ColorType paletteType);
    void newProcessInstance(qint64 pid, const QStringList &arguments);
    void fontChanged(const QFont &font);
    void applicationPaletteChanged();

protected:
    explicit DGuiApplicationHelper();
    virtual void initialize();

private:
    D_PRIVATE_SLOT(void _q_initApplicationTheme(bool))
    friend class _DGuiApplicationHelper;
};

DGUI_END_NAMESPACE

#endif // DGUIAPPLICATIONHELPER_H
