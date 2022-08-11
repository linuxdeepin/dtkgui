// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
