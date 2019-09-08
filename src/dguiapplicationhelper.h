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
    Q_PROPERTY(ColorType paletteType READ paletteType WRITE setPaletteType NOTIFY paletteTypeChanged)

public:
    enum ColorType {
        UnknownType,
        LightType,
        DarkType
    };

    static DGuiApplicationHelper *instance();

    QColor adjustColor(const QColor &base, qint8 hueFloat, qint8 saturationFloat, qint8 lightnessFloat,
                       qint8 redFloat, qint8 greenFloat, qint8 blueFloat, qint8 alphaFloat);
    static QColor blendColor(const QColor &substrate, const QColor &superstratum);
    static DPalette standardPalette(ColorType type);
    static void generatePaletteColor(DPalette &base, QPalette::ColorRole role, ColorType type);
    static void generatePaletteColor(DPalette &palette, DPalette::ColorType role, ColorType type);
    static void generatePalette(DPalette &base, ColorType type = UnknownType);
    static DPalette fetchPalette(const DPlatformTheme *theme);

    DPlatformTheme *systemTheme() const;
    DPlatformTheme *applicationTheme() const;
    DPlatformTheme *windowTheme(QWindow *window) const;

    DPalette applicationPalette() const;
    DPalette windowPalette(QWindow *window) const;

    static ColorType toColorType(const QColor &color);
    static ColorType toColorType(const QPalette &palette);
    ColorType themeType() const;

    ColorType paletteType() const;

public Q_SLOTS:
    void setThemeType(ColorType themeType);
    void setPaletteType(ColorType paletteType);

Q_SIGNALS:
    void themeTypeChanged(ColorType themeType);
    void paletteTypeChanged(ColorType paletteType);

protected:
    explicit DGuiApplicationHelper();
    virtual void initialize();

    typedef DGuiApplicationHelper *(*HelperCreator)();
    static void registerInstanceCreator(HelperCreator creator);

private:
    D_PRIVATE_SLOT(void _q_initApplicationTheme(bool))
    friend class _DGuiApplicationHelper;
};

DGUI_END_NAMESPACE

#endif // DGUIAPPLICATIONHELPER_H
