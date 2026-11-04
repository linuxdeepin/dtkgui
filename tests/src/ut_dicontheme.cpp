// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "DIconTheme"

#include <DStandardPaths>

#include <QDir>
#include <QIcon>

DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

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
    if (!icon2.isNull()) {
        ASSERT_TRUE(DIconTheme::isXdgIcon(icon2));
    }
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
    if (!icon2.isNull()) {
        ASSERT_TRUE(DIconTheme::isXdgIcon(icon2));
    }
#endif

    const QIcon icon1_cached1 = DIconTheme::cached()->findQIcon("edit");
    ASSERT_EQ(icon1_cached1.cacheKey(), icon1.cacheKey());
    DIconTheme::cached()->clear();
    const QIcon icon1_cached2 = DIconTheme::cached()->findQIcon("edit");
    ASSERT_NE(icon1_cached2.cacheKey(), icon1.cacheKey());
}

TEST(ut_DIconTheme, userDciThemePath)
{
    const QString userPath = QDir::cleanPath(DStandardPaths::path(DStandardPaths::XDG::DataHome) + "/dsg/icons");
    const QStringList paths = DIconTheme::dciThemeSearchPaths();
    ASSERT_FALSE(paths.isEmpty());
    // 用户级 DCI 目录优先于系统目录, 且不重复出现
    ASSERT_EQ(QDir::cleanPath(paths.first()), userPath);
    int count = 0;
    for (const QString &path : paths) {
        if (QDir::cleanPath(path) == userPath)
            ++count;
    }
    ASSERT_EQ(count, 1);
}
