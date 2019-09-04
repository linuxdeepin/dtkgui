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
#include "dpalette.h"

DGUI_BEGIN_NAMESPACE

struct DPaletteData : public QSharedData
{
    QBrush br[DPalette::NColorGroups][DPalette::NColorTypes];
};

class DPalettePrivate
{
public:
    DPalettePrivate(const QSharedDataPointer<DPaletteData> &d)
        : data(d)
    {

    }

    QSharedDataPointer<DPaletteData> data;
};

DPalette::DPalette()
    : d(new DPalettePrivate(QSharedDataPointer<DPaletteData>(new DPaletteData())))
{

}

DPalette::DPalette(const QPalette &palette)
    : QPalette(palette)
    , d(new DPalettePrivate(QSharedDataPointer<DPaletteData>(new DPaletteData())))
{

}

DPalette::DPalette(const DPalette &palette)
    : QPalette(palette)
    , d(new DPalettePrivate(palette.d->data))
{

}

DPalette::~DPalette()
{

}

DPalette &DPalette::operator=(const DPalette &palette)
{
    QPalette::operator =(palette);
    d->data = palette.d->data;

    return *this;
}

const QBrush &DPalette::brush(QPalette::ColorGroup cg, DPalette::ColorType cr) const
{
    if (cr >= NColorTypes) {
        return QPalette::brush(cg, QPalette::NoRole);
    }

    if (cg == Current) {
        cg = currentColorGroup();
    } else if (cg >= NColorGroups) {
        cg = Active;
    }

    return d->data->br[cg][cr];
}

void DPalette::setBrush(QPalette::ColorGroup cg, DPalette::ColorType cr, const QBrush &brush)
{
    if (cg == All) {
        for (uint i = 0; i < NColorGroups; i++)
            setBrush(ColorGroup(i), cr, brush);
        return;
    }

    if (cr >= NColorTypes) {
        return QPalette::setBrush(cg, QPalette::NoRole, brush);
    }

    if (cg == Current) {
        cg = currentColorGroup();
    } else if (cg >= NColorGroups) {
        cg = Active;
    }

    d->data->br[cg][cr] = brush;
}

DGUI_END_NAMESPACE

DGUI_USE_NAMESPACE

QT_BEGIN_NAMESPACE
QDataStream &operator<<(QDataStream &ds, const DPalette &p)
{
    ds << static_cast<const QPalette&>(p);

    for (int i = 0; i < DPalette::NColorGroups; ++i) {
        for (int j = 0; j < DPalette::NColorTypes; ++j) {
            ds << p.brush(DPalette::ColorGroup(i), DPalette::ColorType(j));
        }
    }

    return ds;
}

QDataStream &operator>>(QDataStream &ds, DPalette &p)
{
    ds >> static_cast<QPalette&>(p);

    for (int i = 0; i < DPalette::NColorGroups; ++i) {
        for (int j = 0; j < DPalette::NColorTypes; ++j) {
            QBrush brush;
            ds >> brush;

            p.setBrush(DPalette::ColorGroup(i), DPalette::ColorType(j), brush);
        }
    }

    return ds;
}
QT_END_NAMESPACE
