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
#ifndef DPLATFORMTHEME_P_H
#define DPLATFORMTHEME_P_H

#include "dplatformtheme.h"
#include "dnativesettings_p.h"

DGUI_BEGIN_NAMESPACE

class DPlatformThemePrivate : public DNativeSettingsPrivate
{
public:
    D_DECLARE_PUBLIC(DPlatformTheme)
    DPlatformThemePrivate(DPlatformTheme *qq);

    // 接收parent主题或非调色板DNativeSettings对象（theme对象）的属性变化通知
    // 调色板相关的属性变化与此无关
    void _q_onThemePropertyChanged(const QByteArray &name, const QVariant &value);
    void onQtColorChanged(QPalette::ColorRole role, const QColor &color);
    void onDtkColorChanged(DPalette::ColorType type, const QColor &color);
    void notifyPaletteChanged();

    // 父主题，可以从其继承除调色板之外的所有窗口设置
    DPlatformTheme *parent = nullptr;
    // 用于控制是否fallback到父主题中获取属性
    bool fallbackProperty = true;
    // 默认时，DPlatformTheme会从/deepin/palette域下获取调色板相关的属性值
    // 此处的DNativeSettings用于获取除调色板之外的属性设置
    DNativeSettings *theme;
    // 缓存的调色板数据
    DPalette *palette = nullptr;
    // 减少调色板changed信号的通知频率
    QTimer *notifyPaletteChangeTimer = nullptr;
};

DGUI_END_NAMESPACE

#endif // DPLATFORMTHEME_P_H
