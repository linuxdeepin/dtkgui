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
#include "dnativesettings.h"
#include "private/dnativesettings_p.h"

#include <QGuiApplication>
#include <QDebug>

#include <private/qobject_p.h>
#include <private/qmetaobjectbuilder_p.h>

DGUI_BEGIN_NAMESPACE

DNativeSettingsPrivate::DNativeSettingsPrivate(DNativeSettings *qq, const QByteArray &domain)
    : DObjectPrivate(qq)
    , domain(domain)
{

}

DNativeSettingsPrivate::~DNativeSettingsPrivate()
{

}

bool DNativeSettingsPrivate::init(const QMetaObject *mo, quint32 window)
{
    QFunctionPointer native_build_setting_fun = qGuiApp->platformFunction("_d_buildNativeSettings");

    if (!native_build_setting_fun)
        return false;

    D_Q(DNativeSettings);

    if (!domain.isEmpty()) {
        q->setProperty("_d_domain", domain);
    }

    q->setProperty("_d_metaObject", reinterpret_cast<qintptr>(mo));

    return reinterpret_cast<bool(*)(QObject*, quint32)>(native_build_setting_fun)(q, window);
}

/*!
 * \~chinese \class DNativeSettings
 * \~chinese \brief 一个用于本地设置的类
 */
DNativeSettings::DNativeSettings(quint32 window, const QByteArray &domain, QObject *parent)
    : DNativeSettings(&DNativeSettings::staticMetaObject, window, domain, parent)
{

}

bool DNativeSettings::isValid() const
{
    D_DC(DNativeSettings);

    return d->valid;
}

QByteArrayList DNativeSettings::allKeys() const
{
    D_DC(DNativeSettings);

    return d->allKeys;
}

QVariant DNativeSettings::getSetting(const QByteArray &name) const
{
    D_DC(DNativeSettings);

    return property(name.constData());
}

void DNativeSettings::setSetting(const QByteArray &name, const QVariant &value)
{
    D_D(DNativeSettings);

    setProperty(name.constData(), value);
}

DNativeSettings::DNativeSettings(DNativeSettingsPrivate &dd, const QMetaObject *metaObject, quint32 window, QObject *parent)
    : QObject(parent)
    , DObject(dd)
{
    d_func()->valid = init(metaObject, window);
}

DNativeSettings::DNativeSettings(const QMetaObject *metaObject, quint32 window, const QByteArray &domain, QObject *parent)
    : DNativeSettings(*new DNativeSettingsPrivate(this, domain), metaObject, window, parent)
{

}

bool DNativeSettings::init(const QMetaObject *metaObject, quint32 window)
{
    D_D(DNativeSettings);

    return d->init(metaObject, window);
}

void DNativeSettings::__setAllKeys(const QByteArrayList &keys)
{
    D_D(DNativeSettings);

    d->allKeys = keys;

    Q_EMIT allKeysChanged();
}

DGUI_END_NAMESPACE

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug debug, const DTK_GUI_NAMESPACE::DNativeSettings &settings)
{
    const QByteArrayList &keys = settings.allKeys();

    for (const QByteArray &key : keys) {
        debug << key << settings.getSetting(key) << endl;
    }

    return debug;
}
QT_END_NAMESPACE
