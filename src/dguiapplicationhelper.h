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
class DGuiApplicationHelperPrivate;
class DGuiApplicationHelper : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DGuiApplicationHelper)

    Q_PROPERTY(ColorType themeType READ themeType WRITE setThemeType NOTIFY themeTypeChanged)
    Q_PROPERTY(ColorType paletteType READ paletteType WRITE setPaletteType NOTIFY paletteTypeChanged) // 同 themeType

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
    static void setUseInactiveColorGroup(bool on);
    static void setColorCompositingEnabled(bool on);
    static bool isXWindowPlatform();
    static bool isTabletEnvironment();

    DPlatformTheme *systemTheme() const;
    DPlatformTheme *applicationTheme() const;
    D_DECL_DEPRECATED DPlatformTheme *windowTheme(QWindow *window) const;

    DPalette applicationPalette() const;
    void setApplicationPalette(const DPalette &palette);
    D_DECL_DEPRECATED DPalette windowPalette(QWindow *window) const;

    static ColorType toColorType(const QColor &color);
    static ColorType toColorType(const QPalette &palette);
    ColorType themeType() const;
    D_DECL_DEPRECATED_X("Use themeType") ColorType paletteType() const;

    static bool setSingleInstance(const QString &key, SingleScope singleScope = UserScope);
    static void setSingleInstanceInterval(int interval = 3000);
    D_DECL_DEPRECATED static void setSingelInstanceInterval(int interval = 3000);

public Q_SLOTS:
    void setThemeType(ColorType themeType);
    D_DECL_DEPRECATED_X("Use setThemeType") void setPaletteType(ColorType paletteType);

Q_SIGNALS:
    void themeTypeChanged(ColorType themeType);
    D_DECL_DEPRECATED_X("Use themeTypeChanged") void paletteTypeChanged(ColorType paletteType); // 同themeTypeChanged
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
