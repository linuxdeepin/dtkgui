/*
 * Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     Chen Bin <chenbin@uniontech.com>
 *
 * Maintainer: Chen Bin <chenbin@uniontech.com>
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

#include <QSignalSpy>
#include <QDebug>
#include <QGuiApplication>
#include "test.h"
#include "dforeignwindow.h"

DGUI_BEGIN_NAMESPACE

#define WmClass "_d_WmClass"
#define ProcessId "_d_ProcessId"

class TDForeignWindow : public DTest
{
protected:
    virtual void SetUp()
    {

        foreignWindow = new DForeignWindow();
    }
    virtual void TearDown()
    {
        foreignWindow->close();
        delete foreignWindow;
    }

    //QWindow *window = nullptr;
    DForeignWindow * foreignWindow = nullptr;
};

TEST_F(TDForeignWindow, fromWinId)
{
    QWindow *window = new QWindow;

    window->create();
    DForeignWindow *fw = DForeignWindow::fromWinId(window->winId());
    ASSERT_TRUE(fw);
    // qDebug() << "widnowid"  << window->winId() << fw->wmClass() << fw->pid();

    delete fw;
    delete window;
}

TEST_F(TDForeignWindow, wmClass)
{
    foreignWindow->setProperty(WmClass, "d_testwmclass");
    qDebug() << "wmClass" << foreignWindow->wmClass();
}

TEST_F(TDForeignWindow, pid)
{
    __pid_t pid = getpid();
    foreignWindow->setProperty(ProcessId, pid);
    ASSERT_EQ(foreignWindow->pid(), pid);
}

TEST_F(TDForeignWindow, event)
{
    QDynamicPropertyChangeEvent wmevent(WmClass);
    QDynamicPropertyChangeEvent pidevent(ProcessId);

    QSignalSpy wmspy(foreignWindow, SIGNAL(wmClassChanged()));
    ASSERT_TRUE(QGuiApplication::sendEvent(foreignWindow, &wmevent));
    ASSERT_EQ(wmspy.count(), 1);

    QSignalSpy pidspy(foreignWindow, SIGNAL(pidChanged()));
    ASSERT_TRUE(QGuiApplication::sendEvent(foreignWindow, &pidevent));
    ASSERT_EQ(pidspy.count(), 1);
}

DGUI_END_NAMESPACE
