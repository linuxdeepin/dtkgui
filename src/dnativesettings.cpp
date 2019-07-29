/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "dnativesettings.h"
#include "private/dnativesettings_p.h"

#include <QGuiApplication>
#include <QDebug>

#include <private/qobject_p.h>
#include <private/qmetaobjectbuilder_p.h>

DGUI_BEGIN_NAMESPACE

DNativeSettingsPrivate::DNativeSettingsPrivate(DNativeSettings *qq, const QByteArray &domain)
    : QObject(qq)
    , DCORE_NAMESPACE::DObjectPrivate(qq)
    , domain(domain)
{

}

DNativeSettingsPrivate::~DNativeSettingsPrivate()
{
    if (fakeMetaObject) {
        free(fakeMetaObject);
    }
}

static void removeProperty(QMetaObjectBuilder &ob, const QByteArray &name)
{
    int index = ob.indexOfProperty(name);

    if (index >= 0) {
        ob.removeProperty(index);
    }
}

bool DNativeSettingsPrivate::init(const QMetaObject *init, quint32 window)
{
    QFunctionPointer native_build_setting_fun = qGuiApp->platformFunction("_d_buildNativeSettings");

    if (!native_build_setting_fun)
        return false;

    QMetaObjectBuilder ob(init);

    if (!domain.isEmpty())
        ob.addClassInfo("Domain", domain);

    // 移除特殊属性
    removeProperty(ob, "allKeys");

    // 重新构建QMetaObject对象
    fakeMetaObject = ob.toMetaObject();

    return reinterpret_cast<bool(*)(QObject*, quint32)>(native_build_setting_fun)(this, window);
}

const QMetaObject *DNativeSettingsPrivate::metaObject() const
{
    return d_ptr->metaObject ? d_ptr->dynamicMetaObject() : fakeMetaObject;
}

bool DNativeSettingsPrivate::event(QEvent *event)
{
    if (event->type() != QEvent::DynamicPropertyChange) {
        return QObject::event(event);
    }

    QDynamicPropertyChangeEvent *ev = static_cast<QDynamicPropertyChangeEvent*>(event);

    if (QByteArrayLiteral("allKeys") == ev->propertyName()) {
        D_Q(DNativeSettings);

        Q_EMIT q->allKeysChanged();
    }

    return QObject::event(event);
}

DNativeSettings::DNativeSettings(quint32 window, const QByteArray &domain, QObject *parent)
    : DNativeSettings(&DNativeSettings::staticMetaObject, window, domain, parent)
{

}

QByteArrayList DNativeSettings::allKeys() const
{
    D_DC(DNativeSettings);

    return qvariant_cast<QByteArrayList>(d->property("allKeys"));
}

QVariant DNativeSettings::getSetting(const QByteArray &name) const
{
    D_DC(DNativeSettings);

    return d->property(name.constData());
}

void DNativeSettings::setSetting(const QByteArray &name, const QVariant &value)
{
    D_D(DNativeSettings);

    d->setProperty(name.constData(), value);
}

DNativeSettings::DNativeSettings(const QMetaObject *metaObject, quint32 window, const QByteArray &domain, QObject *parent)
    : QObject(parent)
    , DObject(*new DNativeSettingsPrivate(this, domain))
{
    init(metaObject, window);
}

bool DNativeSettings::init(const QMetaObject *metaObject, quint32 window)
{
    D_D(DNativeSettings);

    return d->init(metaObject, window);
}

DNativeSettingsPrivate *DNativeSettings::d_func()
{
    return dynamic_cast<DNativeSettingsPrivate *>(d_d_ptr.data());
}

const DNativeSettingsPrivate *DNativeSettings::d_func() const
{
    return dynamic_cast<const DNativeSettingsPrivate *>(d_d_ptr.data());
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
