/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Chris Xiong <chirs241097@gmail.com>
 *
 * Maintainer: Chris Xiong <chirs241097@gmail.com>
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

#ifndef DFILEDRAGCLIENT_H
#define DFILEDRAGCLIENT_H

#include "dfiledragcommon.h"

#include <dtkgui_global.h>
#include <DObject>

#include <QObject>

class QMimeData;

DGUI_BEGIN_NAMESPACE

class DFileDragClientPrivate;
class DFileDragClient : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
public:
    explicit DFileDragClient(const QMimeData *data, QObject *parent = nullptr);
    int progress() const;
    DFileDragState state() const;

Q_SIGNALS:
    void progressChanged(int progress);
    void stateChanged(DFileDragState state);
    void serverDestroyed();

public:
    static bool checkMimeData(const QMimeData *data);
    static void setTargetData(const QMimeData *data, QString key, QVariant value);
    static void setTargetUrl(const QMimeData *data, QUrl url);

private:
    D_DECLARE_PRIVATE(DFileDragClient)
};

DGUI_END_NAMESPACE

#endif // DFILEDRAGCLIENT_H
