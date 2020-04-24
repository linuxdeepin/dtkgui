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

/*!
 * \~chinese \class DPalette
 * \~chinese \brief DPalette提供了修改的 QPalette 类
 * \~chinese \li DPalette添加了新的颜色类型
 */

/*!
 * \~chinese \enum DPalette::ColorType
 * \~chinese DPalette::ColorType 定义了 DPalette 颜色类型
 * \~chinese \var DPalette::ColorType DPalette::NoType
 * \~chinese 无类型
 * \~chinese \var DPalette::ColorType DPalette::ItemBackground
 * \~chinese 列表项的背景色
 * \~chinese \var DPalette::ColorType DPalette::TextTitle
 * \~chinese 标题型文本的颜色
 * \~chinese \var DPalette::ColorType DPalette::TextTips
 * \~chinese 提示性文本的颜色
 * \~chinese \var DPalette::ColorType DPalette::TextWarning
 * \~chinese 警告类型的文本颜色
* \~chinese \var DPalette::ColorType DPalette::TextLively
 * \~chinese 活跃式文本颜色（不受活动色影响）
 * \~chinese \var DPalette::ColorType DPalette::LightLively
 * \~chinese 活跃式按钮（recommend button）背景色中的亮色（不受活跃色影响）
 * \~chinese \var DPalette::ColorType DPalette::DarkLively
 * \~chinese 活跃式按钮（recommend button）背景色中的暗色，会从亮色渐变到暗色（不受活跃色影响）
 * \~chinese \var DPalette::ColorType DPalette::FrameBorder
 * \~chinese 控件边框颜色
 * \~chinese \var DPalette::ColorType DPalette::NColorTypes
 * \~chinese 无颜色类型
 */

/*!
 * \~chinese \brief DPalette::DPalette构造函数
 */
DPalette::DPalette()
    : d(new DPalettePrivate(QSharedDataPointer<DPaletteData>(new DPaletteData())))
{

}

/*!
 * \~chinese \brief DPalette::DPalette构造函数
 * \~chinese \param palette参数被发送到 QPalette 构造函数
 */
DPalette::DPalette(const QPalette &palette)
    : QPalette(palette)
    , d(new DPalettePrivate(QSharedDataPointer<DPaletteData>(new DPaletteData())))
{

}

/*!
 * \~chinese \brief DPalette::DPalette构造函数
 * \~chinese \param palette参数被发送到 QPalette 构造函数
 */
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

/*!
 * \~chinese \brief DPalette::brush
 * \~chinese \param \sa QPalette::brush()
 */
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

/*!
 * \~chinese \brief DPalette::setBrush设置画刷
 * \~chinese \param \sa cg QPalette::setBrush()
 */
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
