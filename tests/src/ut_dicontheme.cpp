/*
 * Copyright (C) 2021 UnionTech Technology Co., Ltd.
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

#include "test.h"
#include "DIconTheme"

#include <QIcon>

DGUI_USE_NAMESPACE

TEST(ut_DIconTheme, builtinIcon)
{
    const QIcon icon1 = DIconTheme::findQIcon("edit");
    // 内置的图标主题中必然存在 edit 这个图标
    ASSERT_FALSE(icon1.isNull());
    ASSERT_TRUE(DIconTheme::isBuiltinIcon(icon1));

    const QIcon icon2 = DIconTheme::findQIcon("edit", DIconTheme::IgnoreBuiltinIcons);
    // icon2 只可能是从外部找到的图标，不会与 icon1 相同
    ASSERT_TRUE(icon1.cacheKey() != icon2.cacheKey());
#ifndef DTK_DISABLE_LIBXDG
    if (!icon2.isNull())
        ASSERT_TRUE(DIconTheme::isXdgIcon(icon2));
#endif
}

TEST(ut_DIconTheme, cachedTheme)
{
    const QIcon icon1 = DIconTheme::cached()->findQIcon("edit");
    // 内置的图标主题中必然存在 edit 这个图标
    ASSERT_FALSE(icon1.isNull());
    ASSERT_TRUE(DIconTheme::isBuiltinIcon(icon1));

    const QIcon icon2 = DIconTheme::cached()->findQIcon("edit", DIconTheme::IgnoreBuiltinIcons);
    // icon2 只可能是从外部找到的图标，不会与 icon1 相同
    ASSERT_TRUE(icon1.cacheKey() != icon2.cacheKey());
#ifndef DTK_DISABLE_LIBXDG
    if (!icon2.isNull())
        ASSERT_TRUE(DIconTheme::isXdgIcon(icon2));
#endif

    const QIcon icon1_cached1 = DIconTheme::cached()->findQIcon("edit");
    ASSERT_EQ(icon1_cached1.cacheKey(), icon1.cacheKey());
    DIconTheme::cached()->clear();
    const QIcon icon1_cached2 = DIconTheme::cached()->findQIcon("edit");
    ASSERT_NE(icon1_cached2.cacheKey(), icon1.cacheKey());
}
