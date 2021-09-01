/*
   * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co.,Ltd.
   *
   * Author:     chenke <chenke@uniontech.com>
   *
   * Maintainer: chenke <chenke@uniontech.com>
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

#include <QDebug>
#include <QWindow>
#include <QTest>

#define private public
#include "dnativesettings.h"
#undef private
#include "test.h"

class TDNativeSettings : public DTest
{
protected:
    virtual void SetUp()
    {
        window = new QWindow;
        window->create();
        settings = new  DTK_GUI_NAMESPACE::DNativeSettings(quint32(window->winId()));
        settings->setSetting("foo", "bar");
        settings->setSetting("num", 1024);

    }
    virtual void TearDown()
    {
        window->close();
        delete window;
        delete settings;
    }

    QWindow *window = nullptr;
    DTK_GUI_NAMESPACE::DNativeSettings * settings = nullptr;
};

TEST_F(TDNativeSettings, getSetting)
{
    QVariant ne = settings->getSetting("foo");
    ASSERT_TRUE(!ne.toString().compare("bar"));
}

TEST_F(TDNativeSettings, allkeys)
{
    qDebug() << "settings isValid" << settings->isValid();

    settings->__setAllKeys({"foo", "num"});
    // print all keys
    qDebug() << *settings;
}

TEST_F(TDNativeSettings, propertyChanged)
{
    int count = 0;
    QObject::connect(settings, &DTK_GUI_NAMESPACE::DNativeSettings::propertyChanged, [&count](const QByteArray &name, const QVariant &value){
        qDebug() << name << "changed...." << value;
        ++count;
    });

    QObject::connect(settings, &DTK_GUI_NAMESPACE::DNativeSettings::allKeysChanged, [&count](){
        qDebug() << "allKeysChanged....";
        ++count;
    });

    settings->propertyChanged("foo", "bar") ;
    ASSERT_TRUE(QTest::qWaitFor([&]{
        return count == 1;
    }, 1000));

    settings->__setAllKeys({});
    ASSERT_TRUE(QTest::qWaitFor([&]{
        return count == 2;
    }, 1000));
}

