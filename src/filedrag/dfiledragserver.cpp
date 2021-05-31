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

#include "dfiledragserver.h"
#include "private/dfiledragcommon_p.h"

#include <DObjectPrivate>

#include <QMimeData>
#include <QUuid>
#include <QDBusServer>
#include <QDBusConnection>
#include <QDBusVariant>
#include <QDBusError>
#include <QCoreApplication>
#include <QSharedPointer>

DGUI_BEGIN_NAMESPACE

class DDndSourceInterface;

class DFileDragServerPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    QMap<QString, QVariant> data;
    QUuid uuid;

    explicit DFileDragServerPrivate(DFileDragServer *q);
    ~DFileDragServerPrivate();

    void writeMimeData(QMimeData *dest);

    QSharedPointer<DDndSourceInterface> dbusif;
    static QHash<QString, DFileDragServer*> servermap;
    static QWeakPointer<DDndSourceInterface> dbusifinst;

    D_DECLARE_PUBLIC(DFileDragServer)
    friend class DDndSourceInterface;
};

QWeakPointer<DDndSourceInterface> DFileDragServerPrivate::dbusifinst;
QHash<QString, DFileDragServer*> DFileDragServerPrivate::servermap;

class DDndSourceInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DND_INTERFACE)
public:
    explicit DDndSourceInterface(QObject *parent = nullptr) : QObject(parent) {}

Q_SIGNALS:
    void serverDestroyed(QString uuid);
    void stateChanged(QString uuid, int state);
    void progressChanged(QString uuid, int progress);

public Q_SLOTS:
    void setData(QString uuid, QString key, QString value) {
        DFileDragServer *srv = DFileDragServerPrivate::servermap.value(uuid);
        Q_ASSERT(srv);

        DFileDragServerPrivate *d = srv->d_func();

        if (d->data.value(key) != value) {
            d->data[key] = value;
            Q_EMIT srv->targetDataChanged(key);
        }
    }
    int state(QString uuid) const {return m_statemap.value(uuid);}
    int progress(QString uuid) const {return m_progressmap.value(uuid);}

private:
    QHash<QString, int> m_statemap;
    QHash<QString, int> m_progressmap;

    friend class DFileDragServer;
};

/*!
 * \~chinese \class DFileDragServer
 * \~chinese \brief 提供拖拽文件时与文件接收方交互的接口。
 */

/*!
 * \~chinese \fn DFileDragServer::targetDataChanged
 * \~chinese \param
 * \~chinese \brief 信号会在接收方调用 setData 变化时被发送
 */

DFileDragServer::DFileDragServer(QObject *parent)
    : QObject(parent)
    , DCORE_NAMESPACE::DObject(*new DFileDragServerPrivate(this))
{
    D_D(DFileDragServer);
    DFileDragServerPrivate::servermap[d->uuid.toString()] = this;
}

DFileDragServer::~DFileDragServer()
{
    D_D(DFileDragServer);
    DFileDragServerPrivate::servermap.remove(d->uuid.toString());
}

/*!
 * \~chinese \brief DFileDragServer::targetData
 * \~chinese \param key
 * \~chinese \return 返回文件接收方设置数据 key 对应的 value
 */
QVariant DFileDragServer::targetData(const QString &key) const
{
    D_D(const DFileDragServer);

    return d->data.value(key);
}

/*!
 *\~chinese \brief DFileDragServer::setProgress
 *\~chinese \param progress 当前进度
 *\~chinese \brief 拖拽进度更新，接收方会受到 progressChanged 信号
 */
void DFileDragServer::setProgress(int progress)
{
    D_D(DFileDragServer);

    if (d->dbusif && progress != d->dbusif->m_progressmap.value(d->uuid.toString())) {
        d->dbusif->m_progressmap[d->uuid.toString()] = progress;
        Q_EMIT d->dbusif->progressChanged(d->uuid.toString(), progress);
    }
}

/*!
 * \~chinese \brief DFileDragServer::setState
 * \~chinese \param state
 * \~chinese \brief 改变状态，接收方会受到 stateChanged 信号
 */
void DFileDragServer::setState(DFileDragState state)
{
    D_D(DFileDragServer);

    if (d->dbusif && state != d->dbusif->m_statemap.value(d->uuid.toString())) {
        d->dbusif->m_statemap[d->uuid.toString()] = state;
        Q_EMIT d->dbusif->stateChanged(d->uuid.toString(), state);
    }
}

DFileDragServerPrivate::DFileDragServerPrivate(DFileDragServer *q)
    : DCORE_NAMESPACE::DObjectPrivate(q)
    , uuid(QUuid::createUuid())
{
    if (dbusifinst.isNull()) {
        dbusif = QSharedPointer<DDndSourceInterface>(new DDndSourceInterface(), [](DDndSourceInterface *intf){
            QDBusConnection::sessionBus().unregisterObject(DND_OBJPATH);
            intf->deleteLater();
        });
        dbusifinst = dbusif.toWeakRef();
        QDBusConnection::sessionBus().registerObject(DND_OBJPATH, DND_INTERFACE, dbusif.data(), QDBusConnection::RegisterOption::ExportAllContents);
    } else {
        dbusif = dbusifinst.toStrongRef();
    }
}

DFileDragServerPrivate::~DFileDragServerPrivate()
{
    Q_EMIT dbusif->serverDestroyed(uuid.toString());
}

void DFileDragServerPrivate::writeMimeData(QMimeData *dest)
{
    dest->setData(DND_MIME_SERVICE, QDBusConnection::sessionBus().baseService().toUtf8());
    dest->setData(DND_MIME_PID, QString::number(QCoreApplication::applicationPid()).toUtf8());
    dest->setData(DND_MIME_UUID, uuid.toString().toUtf8());
}

DGUI_END_NAMESPACE

#include "dfiledragserver.moc"
