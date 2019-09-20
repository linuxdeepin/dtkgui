/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Chris Xiong <chirs241097@gmail.com>
 *
 * Maintainer: Chris Xiong <chirs241097@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dfiledrag.h"
#include "dfiledragserver.h"
#include "private/dfiledragcommon_p.h"
#include "private/dfiledragserver_p.h"

#include <DObjectPrivate>

#include <QVariant>

DGUI_BEGIN_NAMESPACE

class DFileDragPrivate : DCORE_NAMESPACE::DObjectPrivate
{
    DFileDragPrivate(DFileDrag *q, DFileDragServer *_srv)
        : DObjectPrivate(q)
        , srv(_srv) {}
    DFileDragServer *srv;

    D_DECLARE_PUBLIC(DFileDrag)
};

DFileDrag::DFileDrag(QObject *source, DFileDragServer *server)
    : QDrag(source)
    , DObject(*new DFileDragPrivate(this, server))
{
    Q_D(DFileDrag);

    connect(d->srv, &DFileDragServer::targetDataChanged, this, [this, d](const QString &key) {
        if (key == DND_TARGET_URL_KEY) {
            Q_EMIT this->targetUrlChanged(QUrl(d->srv->targetData(key).value<QString>()));
        }
    });
}

QUrl DFileDrag::targetUrl()
{
    Q_D(DFileDrag);

    return QUrl(d->srv->targetData(DND_TARGET_URL_KEY).value<QString>());
}

void DFileDrag::setMimeData(QMimeData *data)
{
    Q_D(DFileDrag);

    d->srv->d_func()->writeMimeData(data);
    QDrag::setMimeData(data);
}

DGUI_END_NAMESPACE
