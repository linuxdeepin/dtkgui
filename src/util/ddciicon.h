/*
 * Copyright (C) 2021 ~ 2022 UnionTech Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@uniontech.com>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include "ddciiconpalette.h"

#include <dtkgui_global.h>

#include <QPixmap>

DCORE_BEGIN_NAMESPACE
class DDciFile;
DCORE_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

class DDciIconPrivate;
class DDciIcon
{
public:
    enum Mode {
        Normal = 0,
        Disabled = 1,
        Hover = 2,
        Pressed = 3
    };
    enum Theme {
        Light = 0,
        Dark = 1
    };

    DDciIcon();
    explicit DDciIcon(const DCORE_NAMESPACE::DDciFile *dciFile);
    explicit DDciIcon(const QString &fileName);
    explicit DDciIcon(const QByteArray &data);
    DDciIcon(const DDciIcon &other);
    DDciIcon &operator=(const DDciIcon &other) noexcept;
    ~DDciIcon();
    DDciIcon(DDciIcon && other) noexcept;
    DDciIcon &operator=(DDciIcon &&other) noexcept;
    void swap(DDciIcon &other) noexcept { d.swap(other.d); }

    bool isNull() const;
    int actualSize(int size, Theme theme, Mode mode = Normal) const;
    QList<int> availableSizes(Theme theme, Mode mode = Normal) const;

    QPixmap pixmap(qreal devicePixelRatio, int iconSize, Theme theme, Mode mode = Normal,
                   const DDciIconPalette &palette = DDciIconPalette()) const;
    void paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, Theme theme, Mode mode = Normal,
               Qt::Alignment alignment = Qt::AlignCenter, const DDciIconPalette &palette = DDciIconPalette()) const;

    // TODO: Should be compatible with QIcon
private:
    QSharedDataPointer<DDciIconPrivate> d;
};

DGUI_END_NAMESPACE
