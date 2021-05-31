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
    explicit DFileDragClientPrivate(DFileDragClient *q)
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

/*!
 * \~chinese \class DFileDragClient
 * \~chinese \brief 提供拖拽文件时与文件发送方交互的接口。
 */

/*!
 * \~chinese \fn DFileDragClient::progressChanged
 * \~chinese \param 当前进度
 * \~chinese \brief 信号会在当前进度变化时被发送
 * \~chinese \fn DFileDragClient::stateChanged
 * \~chinese \param 改变后的新状态
 * \~chinese \brief 信号会在当前状态变化时被发送
 * \~chinese \fn DFileDragClient::serverDestroyed
 * \~chinese \brief 信号会在发送方析构销毁前被发送
 * \~chinese \note DFileDragClient 收到后会自删除(deletelater)，因此不用去管理 new 出来的 DFileDragClient
 */

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
        QDBusConnection sessionBus(QDBusConnection::sessionBus());
        d->iface = QSharedPointer<QDBusInterface>(new QDBusInterface(d->service, DND_OBJPATH, DND_INTERFACE, sessionBus), [d](QDBusInterface* intf){
            QDBusConnection sessionBus(QDBusConnection::sessionBus());
            sessionBus.disconnect(d->service, DND_OBJPATH, DND_INTERFACE, "progressChanged", "si", d->relay.data(), SLOT(progressChanged(QString, int)));
            sessionBus.disconnect(d->service, DND_OBJPATH, DND_INTERFACE, "stateChanged", "si", d->relay.data(), SLOT(stateChanged(QString, int)));
            sessionBus.disconnect(d->service, DND_OBJPATH, DND_INTERFACE, "serverDestroyed", "s", d->relay.data(), SLOT(serverDestroyed(QString)));
            intf->deleteLater();
            DFileDragClientPrivate::ifacemap.remove(d->service);
        });
        DFileDragClientPrivate::ifacemap[d->service] = d->iface.toWeakRef();
        sessionBus.connect(d->service, DND_OBJPATH, DND_INTERFACE, "progressChanged", "si", d->relay.data(), SLOT(progressChanged(QString, int)));
        sessionBus.connect(d->service, DND_OBJPATH, DND_INTERFACE, "stateChanged", "si", d->relay.data(), SLOT(stateChanged(QString, int)));
        sessionBus.connect(d->service, DND_OBJPATH, DND_INTERFACE, "serverDestroyed", "s", d->relay.data(), SLOT(serverDestroyed(QString)));
    } else {
        d->iface = DFileDragClientPrivate::ifacemap[d->service].toStrongRef();
    }

}

/*!
 * \~chinese \brief DFileDragClient::progress
 * \~chinese \return 返回当前拖拽的进度
 */
int DFileDragClient::progress() const
{
    D_D(const DFileDragClient);

    return QDBusReply<int>(d->iface->call("progress", d->uuid.toString())).value();
}

/*!
 * \~chinese \brief DFileDragClient::state
 * \~chinese \return 返回当前状态,见 DFileDragState
 */
DFileDragState DFileDragClient::state() const
{
    D_D(const DFileDragClient);

    return static_cast<DFileDragState>(QDBusReply<int>(d->iface->call("state", d->uuid.toString())).value());
}

/*!
 * \~chinese \brief DFileDragClient::checkMimeData
 * \~chinese \param data
 * \~chinese \return 包含 DND_MIME_PID 格式的数据时返回 true，否则返回 false
 * \~chinese \note 通常在接收拖放数据的应用的dropEvent(QDropEvent *event)函数中检测当前 event->mimeData() 是否是 DFileDrag
 */
bool DFileDragClient::checkMimeData(const QMimeData *data)
{
    return data->hasFormat(DND_MIME_SERVICE) && data->hasFormat(DND_MIME_PID);
}

/*!
 * \~chinese \brief DFileDragClient::setTargetData
 * \~chinese \param data 拖放时传入的data,用于获取和发送数据的应用dbus通讯需要的一些信息
 * \~chinese \param key
 * \~chinese \param value
 * \~chinese \note 向文件发送方设置自定义数据
 */
void DFileDragClient::setTargetData(const QMimeData *data, QString key, QVariant value)
{
    Q_ASSERT(checkMimeData(data));
    QString service(data->data(DND_MIME_SERVICE));
    QString uuid(data->data(DND_MIME_UUID));
    QDBusInterface iface(service, DND_OBJPATH, DND_INTERFACE, QDBusConnection::sessionBus());

    QDBusReply<uint> pid = QDBusConnection::sessionBus().interface()->servicePid(service);
    if (QString::number(pid).toUtf8() != data->data(DND_MIME_PID)) {
        return;
    }
    iface.call("setData", uuid, key, value.toString());
}

/*!
 * \~chinese \brief DFileDragClient::setTargetUrl
 * \~chinese \param data
 * \~chinese \param url
 * \~chinese \note 告知文件发送方拖拽目标路径
 */
void DFileDragClient::setTargetUrl(const QMimeData *data, QUrl url)
{
    setTargetData(data, DND_TARGET_URL_KEY, QVariant::fromValue(url.toString()));
}

DGUI_END_NAMESPACE

#include "dfiledragclient.moc"
