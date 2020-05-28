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
#ifndef DGUIAPPLICATIONHELPER_P_H
#define DGUIAPPLICATIONHELPER_P_H

#include "dguiapplicationhelper.h"
#include "dplatformtheme.h"

#include <DObjectPrivate>

QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

class DGuiApplicationHelperPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
public:
    D_DECLARE_PUBLIC(DGuiApplicationHelper)

    DGuiApplicationHelperPrivate(DGuiApplicationHelper *qq);
    void init();
    void initApplication(QGuiApplication *app);
    static void staticInitApplication();
    DPlatformTheme *initWindow(QWindow *window) const;
    void _q_initApplicationTheme(bool notifyChange = false);
    void notifyAppThemeChanged(QGuiApplication *app, bool ignorePaletteType = false);

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::UnknownType;
    DGuiApplicationHelper::ColorType paletteType = DGuiApplicationHelper::UnknownType;
    // 系统级别的主题设置
    DPlatformTheme *systemTheme;
    // 应用程序级别的主题设置
    DPlatformTheme *appTheme = nullptr;
    DPalette *appPalette = nullptr;
    static bool useInactiveColor;
    // 是否采用半透明样式的调色板
    static bool compositingColor;
    // 获取QLocalSever消息的等待时间
    static int waitTime;
};

DGUI_END_NAMESPACE

#endif // DGUIAPPLICATIONHELPER_P_H
