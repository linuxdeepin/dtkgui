/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
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

#define private public
#include <QFont>
#undef private

#include <DGuiApplicationHelper>

#include <QGuiApplication>

#include "dfontmanager.h"
#include "private/dfontmanager_p.h"

#include <private/qfont_p.h>

#include <cmath>

DGUI_BEGIN_NAMESPACE

static DFontManager *g_instance = nullptr;

DFontManagerPrivate::DFontManagerPrivate(DFontManager *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
{
    fontPixelSizeDiff = static_cast<quint16>(DFontManager::fontPixelSize(qGuiApp->font()) - fontPixelSize[DFontManager::T6]);
}

/*!
 * \~chinese \class DFontManager
 * \~chinese \brief 字体大小设置的一个类,系统默认只设置T6
 */

DFontManager::DFontManager(QObject *parent)
    : QObject(parent)
    , DTK_CORE_NAMESPACE::DObject(*new DFontManagerPrivate(this))
{
}

DFontManager::~DFontManager()
{
}

// instance 返回的实例用于全局的字体管理，public 的构造函数用于给用户自行分配管理字体
DFontManager *DFontManager::instance()
{
    if (!g_instance) {
        g_instance = new DFontManager;

        // 处理instance返回实例的信号处理
        connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::fontChanged, g_instance, [](const QFont &font) {
            g_instance->setFontGenericPixelSize(static_cast<quint16>(DFontManager::fontPixelSize(font)));
        });
    }

    return g_instance;
}

/*!
 * \~chinese \enum DFontManager::SizeType
 * \~chinese DFontManager::SizeType 定义了 DFontManager 的系统字体的定义的大小; 而系统只会设置 T6 为系统默认的字体
 * \~chinese \var DFontManager:SizeType DFontManager::T1
 * \~chinese 系统级别为 T1 的字体大小, 默认是40 px
 * \~chinese \var DFontManager:SizeType DFontManager::T2
 * \~chinese 系统级别为 T2 的字体大小, 默认是30 px
 * \~chinese \var DFontManager:SizeType DFontManager::T3
 * \~chinese 系统级别为 T3 的字体大小, 默认是24 px
 * \~chinese \var DFontManager:SizeType DFontManager::T4
 * \~chinese 系统级别为 T4 的字体大小, 默认是20 px
 * \~chinese \var DFontManager:SizeType DFontManager::T5
 * \~chinese 系统级别为 T5 的字体大小, 默认是17 px
 * \~chinese \var DFontManager:SizeType DFontManager::T6
 * \~chinese 系统级别为 T6 的字体大小, 默认是14 px
 * \~chinese \var DFontManager:SizeType DFontManager::T7
 * \~chinese 系统级别为 T7 的字体大小, 默认是13 px
 * \~chinese \var DFontManager:SizeType DFontManager::T8
 * \~chinese 系统级别为 T8 的字体大小, 默认是12 px
 * \~chinese \var DFontManager:SizeType DFontManager::T9
 * \~chinese 系统级别为 T9 的字体大小, 默认是11 px
 * \~chinese \var DFontManager:SizeType DFontManager::T10
 * \~chinese 系统级别为 T10 的字体大小, 默认是10 px
 */

/*!
 * \~chinese \brief 将字体的大小枚举 SizeType 和控件 widget 进行绑定, 其控件的字体大小(随绑定的枚举的)对应值的改变而改变;
 * \~chinese        系统自定义的绑定枚举值 T6, 若 T6 = 14px,  则其他枚举 T1 - T10 的数值,依次为:40, 30, 24, 20, 17, 14(T6), 13, 12, 11, 10;
 * \~chinese        系统自定义的绑定枚举值 T6 改为 T6 = 20px, 则其他枚举 T1 - T10 的数值,依次为:46, 36, 30, 26, 23, 20(T6), 19, 18, 17, 16;
 * \~chinese        即: 其对应的无论 T6 为何值, 其两个相邻的 T 值的差是定值: T(n) - T(n-1) == 定值
 * \~chinese        而系统是只设置 T6 这以枚举, 作为基准
 * \~chinese \param[in] widget 将要绑定字体大小枚举数值的控件
 * \~chinese \param[int] type 字体的枚举类型, 每一个枚举数值对应着一个字体像素大小
 */

/*!
 * \~chinese \brief 获取字体像素的大小
 * \~chinese \param[in] type 字体枚举类型
 * \~chinese \return 返回字体像素的大小
 */
quint16 DFontManager::fontPixelSize(DFontManager::SizeType type) const
{
    D_DC(DFontManager);

    if (type >= NSizeTypes) {
        return 0;
    }

    return d->fontPixelSize[type] + d->fontPixelSizeDiff;
}

/*!
 * \~chinese \brief 设置字体像素大小
 * \~chinese \param[in] type 字体枚举类型
 * \~chinese \param[in] size 字体大小
 */
void DFontManager::setFontPixelSize(DFontManager::SizeType type, quint16 size)
{
    D_D(DFontManager);

    if (type >= NSizeTypes) {
        return;
    }

    if (d->fontPixelSize[type] == size) {
        return;
    }

    d->fontPixelSize[type] = size;
}

/*!
 * \~chinese \brief 设置字体的通用像素大小
 * \~chinese \param[in] size 预设计的字体像素的大小
 */
void DFontManager::setFontGenericPixelSize(quint16 size)
{
    D_D(DFontManager);

    quint16 diff = size - d->fontPixelSize[d->fontGenericSizeType];

    if (diff == d->fontPixelSizeDiff)
        return;

    d->fontPixelSizeDiff = diff;
    Q_EMIT fontGenericPixelSizeChanged();
}

quint16 DFontManager::fontGenericPixelSize() const
{
    D_DC(DFontManager);

    return d->fontPixelSizeDiff;
}

const QFont DFontManager::get(DFontManager::SizeType type, const QFont &base) const
{
    return get(type, base.weight(), base);
}

/*!
 * \~chinese \brief 获取字体
 * \~chinese \param[in] type 字体的大小枚举
 * \~chinese \param[in] base 将改变大小的字体
 * \~chinese \return 返回设置字体大小后的字体
 */
const QFont DFontManager::get(DFontManager::SizeType type, int weight, const QFont &base) const
{
    QFont font = base;

    font.setPixelSize(fontPixelSize(type));
    font.setWeight(weight);

    return font;
}

int DFontManager::fontPixelSize(const QFont &font)
{
    int px = font.pixelSize();

    if (px == -1) {
        px = qRound(std::floor(((font.pointSizeF() * font.d->dpi) / 72) * 100 + 0.5) / 100);
    }

    return px;
}

DGUI_END_NAMESPACE
