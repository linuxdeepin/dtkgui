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

#ifndef DWINDOWGROUPLEADER_H
#define DWINDOWGROUPLEADER_H

#include <dtkgui_global.h>

#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

class DWindowGroupLeaderPrivate;
class DWindowGroupLeader
{
public:
    explicit DWindowGroupLeader(quint32 groupId = 0);
    ~DWindowGroupLeader();

    quint32 groupLeaderId() const;
    quint32 clientLeaderId() const;

    void addWindow(QWindow *window);
    void removeWindow(QWindow *window);

private:
    QScopedPointer<DWindowGroupLeaderPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DWindowGroupLeader)
};

DGUI_END_NAMESPACE

#endif // DWINDOWGROUPLEADER_H
