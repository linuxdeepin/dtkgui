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
#include "test.h"
#include "dwindowgroupleader.h"
#include <QDebug>

#include <QTest>
#include <QWindow>

DGUI_USE_NAMESPACE


class TDWindowGroupLeader : public DTest
{
protected:
    void SetUp();
    void TearDown();

    DWindowGroupLeader *groupLeader;
    QWindow *window1;
    QWindow *window2;
};

void TDWindowGroupLeader::SetUp()
{
    groupLeader = new DWindowGroupLeader;
    window1 = new QWindow;
    window2 = new QWindow;
}

void TDWindowGroupLeader::TearDown()
{
    delete groupLeader;
    delete window1;
    delete window2;
}

TEST_F(TDWindowGroupLeader, testFunctions)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    // 测试多种情况添加window时数据是否正常
    groupLeader->addWindow(window1);
    groupLeader->addWindow(window2);

    ASSERT_TRUE(groupLeader->groupLeaderId() != 0);
    ASSERT_TRUE(groupLeader->clientLeaderId() != 0);

    groupLeader->removeWindow(window1);

    ASSERT_TRUE(groupLeader->groupLeaderId() != 0);
    ASSERT_TRUE(groupLeader->clientLeaderId() != 0);

    groupLeader->addWindow(window2);
    ASSERT_TRUE(groupLeader->groupLeaderId() != 0);
    ASSERT_TRUE(groupLeader->clientLeaderId() != 0);
}
