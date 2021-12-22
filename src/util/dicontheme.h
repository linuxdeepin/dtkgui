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
#ifndef DICONTHEME_H
#define DICONTHEME_H

#include <dtkgui_global.h>

#include <QScopedPointer>
#include <QIcon>

DGUI_BEGIN_NAMESPACE

namespace DIconTheme
{
    enum Option {
        DontFallbackToQIconFromTheme = 1 << 0,
        IgnoreBuiltinIcons = 1 << 1
    };
    Q_DECLARE_FLAGS(Options, Option)

    class CachedData;
    class Cached {
        CachedData *data = nullptr;
    public:
        Cached();
        ~Cached();

        int maxCost() const;
        void setMaxCost(int cost);
        void clear();

        QIcon findQIcon(const QString &iconName, Options options = Options(), const QIcon &fallback = QIcon());
        QString findDciIconFile(const QString &iconName, const QString &themeName, const QString &fallback = {});
    };

    Cached *cached();
    QIcon findQIcon(const QString &iconName, Options options = Options());

    QString findDciIconFile(const QString &iconName, const QString &themeName);

    QStringList dciThemeSearchPaths();
    void setDciThemeSearchPaths(const QStringList &path);

    bool isBuiltinIcon(const QIcon &icon);
    bool isXdgIcon(const QIcon &icon);
}

DGUI_END_NAMESPACE

#endif // DICONTHEME_H
