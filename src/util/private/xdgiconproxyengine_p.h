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
#ifndef XDGICONPROXYENGINE_H
#define XDGICONPROXYENGINE_H

#include <dtkgui_global.h>

#include <QIconEngine>
#if XDG_ICON_VERSION_MAR >= 3
#define private public
#include <private/xdgiconloader/xdgiconloader_p.h>
#undef private
#elif XDG_ICON_VERSION_MAR == 2
//这个版本中的xdgiconloader_p.h定义和qiconloader_p.h有冲突
//只能通过此方式提供创建XdgIconLoaderEngine对象的接口
#include "xdgiconenginecreator.h"
#endif

class ScalableEntry;
class QIconLoaderEngineEntry;

DGUI_BEGIN_NAMESPACE

#if XDG_ICON_VERSION_MAR >= 3
class Q_DECL_HIDDEN XdgIconProxyEngine : public QIconEngine
{
public:
    XdgIconProxyEngine(XdgIconLoaderEngine *proxy);
    virtual ~XdgIconProxyEngine() override;

    static quint64 entryCacheKey(const ScalableEntry *color_entry, const QIcon::Mode mode, const QIcon::State state);

    QPixmap followColorPixmap(ScalableEntry *color_entry, const QSize &size, QIcon::Mode mode, QIcon::State state);

    QPixmap pixmapByEntry(QIconLoaderEngineEntry *entry, const QSize &size, QIcon::Mode mode, QIcon::State state);
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state) override;
    void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QString key() const override;
    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine *clone() const override;
    bool read(QDataStream &in) override;
    bool write(QDataStream &out) const override;
    void virtual_hook(int id, void *data) override;

private:
    XdgIconLoaderEngine *engine;
    QHash<quint64, QString> entryToColorScheme;
    QIcon::Mode lastMode;
};
#endif

DGUI_END_NAMESPACE

#endif // XDGICONPROXYENGINE_H
