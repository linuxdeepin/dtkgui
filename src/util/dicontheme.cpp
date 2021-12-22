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
#include "dicontheme.h"
#include "private/dbuiltiniconengine_p.h"
#ifndef DTK_DISABLE_LIBXDG
#include "private/xdgiconproxyengine_p.h"
#endif

#include <DStandardPaths>

#include <QCache>
#include <QSet>
#include <QGuiApplication>
#include <QFileInfo>
#include <QDir>

DGUI_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

static QString joinPath(const QString &basePath, const QString &path)
{
    if (path.isEmpty())
        return basePath;

    return basePath + QDir::separator() + path;
}

static inline QString systemDciThemePath()
{
    return joinPath(DStandardPaths::path(DStandardPaths::DSG::DataDir), QLatin1String("icons"));
}

static inline QString applicationDciThemePath()
{
    return QLatin1String(":/dsg/icons");
}

static inline QString applicationBuiltInIconPath()
{
    return QLatin1String(":/dsg/built-in-icons");
}

static QString findDciIconFromPath(const QString &iconName, const QString &themeName, const QString path)
{
    if (path.isEmpty())
        return nullptr;

    QString themePath = joinPath(path, themeName);
    QFileInfo themeInfo(themePath);
    if (!themeInfo.exists() || !themeInfo.isDir())
        return nullptr;

    /*
     *  iconName has two types, like as:
     *  1. example.dci
     *  2. org.deepin.app/example.dci
     *
     *  We do not need to consider the icon name,
     *  just whether the file exists.
     */
    QString iconNameWithSuffix(iconName);
    // Focus that the icon name can not contain the dci suffix.
    iconNameWithSuffix += QLatin1String(".dci");
    QString iconPath = joinPath(themePath, iconNameWithSuffix);
    if (!QDir::cleanPath(iconPath).startsWith(QDir::cleanPath(themePath)))  // Wrongful
        return nullptr;

    QFileInfo iconInfo(iconPath);
    if (iconInfo.exists() && iconInfo.isFile())
        return iconPath;

    return nullptr;
}

static inline QIconEngine *createBuiltinIconEngine(const QString &iconName)
{
    return new DBuiltinIconEngine(iconName);
}

#ifndef DTK_DISABLE_LIBXDG
static inline QIconEngine *createXdgProxyIconEngine(const QString &iconName)
{
    return new XdgIconProxyEngine(new XdgIconLoaderEngine(iconName));
}
#endif

QIcon DIconTheme::findQIcon(const QString &iconName, Options options)
{
    if (Q_UNLIKELY(!options.testFlag(IgnoreBuiltinIcons))) {
        thread_local static QSet<QString> non_builtin_icon_cache;

        if (!non_builtin_icon_cache.contains(iconName)) {
            // 记录下来此种类型的icon为内置图标
            // 因此，此处添加的缓存不考虑更新
            // 优先使用内置图标
            if (QIconEngine *engine = createBuiltinIconEngine(iconName)) {
                if (engine->isNull()) {
                    non_builtin_icon_cache.insert(iconName);
                    delete engine;
                } else {
                    return QIcon(engine);
                }
            } else {
                non_builtin_icon_cache.insert(iconName);
            }
        }
    }

#ifdef DTK_DISABLE_LIBXDG
    if (options.testFlag(DontFallbackToQIconFromTheme))
        return QIcon();
    return QIcon::fromTheme(iconName);
#else
    Q_UNUSED(options)
    return QIcon(createXdgProxyIconEngine(iconName));
#endif
}

bool DIconTheme::isBuiltinIcon(const QIcon &icon)
{
    if (icon.isNull())
        return false;
    return typeid(*const_cast<QIcon&>(icon).data_ptr()->engine) == typeid(DBuiltinIconEngine);
}

bool DIconTheme::isXdgIcon(const QIcon &icon)
{
#ifdef DTK_DISABLE_LIBXDG
    return false;
#else
    if (icon.isNull())
        return false;
    return typeid(*const_cast<QIcon&>(icon).data_ptr()->engine) == typeid(XdgIconProxyEngine);
#endif
}

class DIconTheme::CachedData
{
public:
    QCache<QString, QIcon> cache;
    QCache<QString, QString> dciIconPathCache;
};

DIconTheme::Cached::Cached()
    : data(new CachedData())
{

}

DIconTheme::Cached::~Cached()
{
    delete data;
}

int DIconTheme::Cached::maxCost() const
{
    return data->cache.maxCost();
}

void DIconTheme::Cached::setMaxCost(int cost)
{
    data->cache.setMaxCost(cost);
    data->dciIconPathCache.setMaxCost(cost);
}

void DIconTheme::Cached::clear()
{
    data->cache.clear();
    data->dciIconPathCache.clear();
}

QIcon DIconTheme::Cached::findQIcon(const QString &iconName, Options options, const QIcon &fallback)
{
    const QString cacheKey = iconName + QChar('/') + QString::number(static_cast<int>(options));
    if (data->cache.contains(cacheKey)) {
        auto cacheIcon = data->cache.object(cacheKey);
        if (cacheIcon->isNull())
            return fallback;
        return *cacheIcon;
    }

    auto newIcon = new QIcon(DIconTheme::findQIcon(iconName, options));
    data->cache.insert(cacheKey, newIcon);

    if (newIcon->isNull())
        return fallback;

    return *newIcon;
}

QString DIconTheme::Cached::findDciIconFile(const QString &iconName, const QString &themeName, const QString &fallback)
{
    const QString cacheKey = themeName + QLatin1Char('/') + iconName;
    if (data->dciIconPathCache.contains(cacheKey)) {
        const QString *cachePath = data->dciIconPathCache.object(cacheKey);
        if (cachePath->isEmpty())
            return fallback;
        return *cachePath;
    }

    QString *path = new QString(DIconTheme::findDciIconFile(iconName, themeName));
    data->dciIconPathCache.insert(cacheKey, path);

    if (path->isEmpty())
        return fallback;

    return *path;
}

Q_GLOBAL_STATIC(DIconTheme::Cached, _globalCache)
Q_GLOBAL_STATIC_WITH_ARGS(QStringList, _dciThemePath, ({systemDciThemePath(), applicationDciThemePath()}))

static void cleanGlobalCache() {
    if (_globalCache.exists())
        _globalCache->clear();
}

DIconTheme::Cached *DIconTheme::cached()
{
    if (Q_UNLIKELY(!_globalCache.exists() && !_globalCache.isDestroyed())) {
        qAddPostRoutine(cleanGlobalCache);
    }
    return _globalCache();
}

QString DIconTheme::findDciIconFile(const QString &iconName, const QString &themeName)
{
    if (iconName.isEmpty() || themeName.isEmpty())
        return nullptr;

    const QString cleanIconName = QDir::cleanPath(iconName);
    if (iconName.startsWith('/') || iconName.endsWith('/')
            || cleanIconName.length() != iconName.length()
            || cleanIconName.startsWith("../"))  // Wrongful
        return nullptr;

    for (const QString &themePath : DIconTheme::dciThemeSearchPaths()) {
        QString iconPath = findDciIconFromPath(iconName, themeName, themePath);
        if (!iconPath.isEmpty())
            return iconPath;
    }

    return findDciIconFromPath(iconName, nullptr, applicationBuiltInIconPath());
}

QStringList DIconTheme::dciThemeSearchPaths()
{
    return *_dciThemePath;
}

void DIconTheme::setDciThemeSearchPaths(const QStringList &path)
{
    *_dciThemePath = path;
}

DGUI_END_NAMESPACE
