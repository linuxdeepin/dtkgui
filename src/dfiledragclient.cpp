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

#include "dfiledragclient.h"
#include "private/dfiledragcommon_p.h"

#include <DObjectPrivate>

#include <QMimeData>
#include <QUrl>
#include <QUuid>
#include <QSharedPointer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>

DGUI_BEGIN_NAMESPACE

class DDndClientSignalRelay : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void progressChanged(QString uuid, int progress);
    void stateChanged(QString uuid, int state);
    void serverDestroyed(QString uuid);

private:
    static QWeakPointer<DDndClientSignalRelay> relayref;
    friend class DFileDragClient;
};

class DFileDragClientPrivate : DCORE_NAMESPACE::DObjectPrivate
{
    DFileDragClientPrivate(DFileDragClient *q)
        : DCORE_NAMESPACE::DObjectPrivate(q) {}
    QUuid uuid;
    QString service;
    QSharedPointer<QDBusInterface> iface;
    QSharedPointer<DDndClientSignalRelay> relay;

    static QHash<QString, DFileDragClient*> connectionmap;
    static QHash<QString, QWeakPointer<QDBusInterface>> ifacemap;

    D_DECLARE_PUBLIC(DFileDragClient)
    friend class DDndClientSignalRelay;
};

QHash<QString, DFileDragClient*> DFileDragClientPrivate::connectionmap;
QHash<QString, QWeakPointer<QDBusInterface>> DFileDragClientPrivate::ifacemap;
QWeakPointer<DDndClientSignalRelay> DDndClientSignalRelay::relayref;

void DDndClientSignalRelay::progressChanged(QString uuid, int progress)
{
    if (DFileDragClientPrivate::connectionmap.contains(uuid)) {
        Q_EMIT DFileDragClientPrivate::connectionmap[uuid]->progressChanged(progress);
    }
}
void DDndClientSignalRelay::stateChanged(QString uuid, int state)
{
    if (DFileDragClientPrivate::connectionmap.contains(uuid)) {
        Q_EMIT DFileDragClientPrivate::connectionmap[uuid]->stateChanged(static_cast<DFileDragState>(state));
    }
}
void DDndClientSignalRelay::serverDestroyed(QString uuid)
{
    if (DFileDragClientPrivate::connectionmap.contains(uuid)) {
        DFileDragClientPrivate::connectionmap[uuid]->deleteLater();
        DFileDragClientPrivate::connectionmap.remove(uuid);
    }
}

DFileDragClient::DFileDragClient(const QMimeData *data, QObject *parent)
    : QObject(parent)
    , DCORE_NAMESPACE::DObject(*new DFileDragClientPrivate(this))
{
    D_D(DFileDragClient);
    Q_ASSERT(checkMimeData(data));

    d->service = data->data(DND_MIME_SERVICE);
    d->uuid = QUuid(data->data(DND_MIME_UUID));
    d->connectionmap[d->uuid.toString()] = this;

    if (DDndClientSignalRelay::relayref.isNull()) {
        d->relay = QSharedPointer<DDndClientSignalRelay>(new DDndClientSignalRelay);
        DDndClientSignalRelay::relayref = d->relay.toWeakRef();
    } else {
        d->relay = DDndClientSignalRelay::relayref.toStrongRef();
    }

    if (!DFileDragClientPrivate::ifacemap.contains(d->service)) {
        QDBusConnection sysbus(QDBusConnection::systemBus());
        d->iface = QSharedPointer<QDBusInterface>(new QDBusInterface(d->service, DND_OBJPATH, DND_INTERFACE, sysbus), [d](QDBusInterface* intf){
            QDBusConnection sysbus(QDBusConnection::systemBus());
            sysbus.disconnect(d->service, DND_OBJPATH, DND_INTERFACE, "progressChanged", "si", d->relay.data(), SLOT(progressChanged(QString, int)));
            sysbus.disconnect(d->service, DND_OBJPATH, DND_INTERFACE, "stateChanged", "si", d->relay.data(), SLOT(stateChanged(QString, int)));
            sysbus.disconnect(d->service, DND_OBJPATH, DND_INTERFACE, "serverDestroyed", "s", d->relay.data(), SLOT(serverDestroyed(QString)));
            intf->deleteLater();
            DFileDragClientPrivate::ifacemap.remove(d->service);
        });
        DFileDragClientPrivate::ifacemap[d->service] = d->iface.toWeakRef();
        sysbus.connect(d->service, DND_OBJPATH, DND_INTERFACE, "progressChanged", "si", d->relay.data(), SLOT(progressChanged(QString, int)));
        sysbus.connect(d->service, DND_OBJPATH, DND_INTERFACE, "stateChanged", "si", d->relay.data(), SLOT(stateChanged(QString, int)));
        sysbus.connect(d->service, DND_OBJPATH, DND_INTERFACE, "serverDestroyed", "s", d->relay.data(), SLOT(serverDestroyed(QString)));
    } else {
        d->iface = DFileDragClientPrivate::ifacemap[d->service].toStrongRef();
    }

}

int DFileDragClient::progress() const
{
    D_D(const DFileDragClient);

    return QDBusReply<int>(d->iface->call("progress", d->uuid.toString())).value();
}

DFileDragState DFileDragClient::state() const
{
    D_D(const DFileDragClient);

    return static_cast<DFileDragState>(QDBusReply<int>(d->iface->call("state", d->uuid.toString())).value());
}

bool DFileDragClient::checkMimeData(const QMimeData *data)
{
    return data->hasFormat(DND_MIME_SERVICE) && data->hasFormat(DND_MIME_PID);
}

void DFileDragClient::setTargetData(const QMimeData *data, QString key, QVariant value)
{
    Q_ASSERT(checkMimeData(data));
    QString service(data->data(DND_MIME_SERVICE));
    QString uuid(data->data(DND_MIME_UUID));
    QDBusInterface iface(service, DND_OBJPATH, DND_INTERFACE, QDBusConnection::systemBus());

    QDBusReply<uint> pid = QDBusConnection::systemBus().interface()->servicePid(service);
    if (QString::number(pid).toUtf8() != data->data(DND_MIME_PID)) {
        return;
    }
    iface.call("setData", uuid, key, value.toString());
}

void DFileDragClient::setTargetUrl(const QMimeData *data, QUrl url)
{
    setTargetData(data, DND_TARGET_URL_KEY, QVariant::fromValue(url.toString()));
}

DGUI_END_NAMESPACE

#include "dfiledragclient.moc"
