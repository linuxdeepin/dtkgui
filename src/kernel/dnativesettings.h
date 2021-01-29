/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
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
#ifndef DNATIVESETTINGS_H
#define DNATIVESETTINGS_H

#include <dtkgui_global.h>
#include <DObject>

#include <QObject>

DGUI_BEGIN_NAMESPACE

class DNativeSettingsPrivate;
class DNativeSettings : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DNativeSettings)
    Q_PROPERTY(QByteArrayList allKeys READ allKeys WRITE __setAllKeys NOTIFY allKeysChanged)
public:
    explicit DNativeSettings(quint32 window, const QByteArray &domain = QByteArray(), QObject *parent = nullptr);

    bool isValid() const;
    QByteArrayList allKeys() const;

    QVariant getSetting(const QByteArray &name) const;
    void setSetting(const QByteArray &name, const QVariant &value);

Q_SIGNALS:
    void allKeysChanged();
    void propertyChanged(const QByteArray &name, const QVariant &value);

protected:
    DNativeSettings(DNativeSettingsPrivate &dd, const QMetaObject *metaObject, quint32 window, QObject *parent);
    DNativeSettings(const QMetaObject *metaObject, quint32 window, const QByteArray &domain, QObject *parent);

    bool init(const QMetaObject *metaObject, quint32 window);

private:
    void __setAllKeys(const QByteArrayList &keys);

    friend class DNativeSettingsPrivate;
};

DGUI_END_NAMESPACE

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug debug, const DTK_GUI_NAMESPACE::DNativeSettings &settings);
QT_END_NAMESPACE

#endif // DNATIVESETTINGS_H
