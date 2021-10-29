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
#ifndef DBUILTINICONENGINE_H
#define DBUILTINICONENGINE_H

#include <dtkgui_global.h>

#include <QIconEngine>
#include <private/qiconloader_p.h>

DGUI_BEGIN_NAMESPACE

// 内置的主题引擎，会从Qt资源文件中查找图标
class DBuiltinIconEnginePrivate;
class Q_DECL_HIDDEN DBuiltinIconEngine : public QIconEngine
{
public:
    DBuiltinIconEngine(const QString &iconName);
    ~DBuiltinIconEngine();
    void paint(QPainter *painter, const QRect &rect,
               QIcon::Mode mode, QIcon::State state);
    QSize actualSize(const QSize &size, QIcon::Mode mode,
                     QIcon::State state);
    QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                   QIcon::State state);

    QString key() const;
    QIconEngine *clone() const;
    bool read(QDataStream &in);
    bool write(QDataStream &out) const;

    QString iconName() const override;

    static QThemeIconInfo loadIcon(const QString &iconName, uint key);

private:
    bool hasIcon() const;
    void ensureLoaded();
    void virtual_hook(int id, void *data) override;

    DBuiltinIconEngine(const DBuiltinIconEngine &other);
    QThemeIconInfo m_info;
    QString m_iconName;
    // 图标的类型(Dark/Light)
    uint m_key:2;
    // 记录是否已经初始化
    bool m_initialized:1;
    // 控制是否跟随系统级别的主题色来改变图标类型
    bool m_followSystemTheme:1;

    friend class QIconLoader;
};

DGUI_END_NAMESPACE

#endif // DBUILTINICONENGINE_H
