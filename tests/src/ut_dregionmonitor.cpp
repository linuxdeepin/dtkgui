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
#include "dregionmonitor.h"
#include "private/dregionmonitor_p.h"

#include <QRegion>
#include <QSignalSpy>
#include <QApplication>

DGUI_BEGIN_NAMESPACE

class TDRegionMonitor : public DTest
{
protected:
    virtual void SetUp()
    {
        regionMonitor = new DRegionMonitor(nullptr);
    }
    virtual void TearDown()
    {
        delete regionMonitor;
    }
    DRegionMonitor *regionMonitor = nullptr;
};

TEST_F(TDRegionMonitor, registerRegion)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    regionMonitor->registerRegion();
    ASSERT_TRUE(regionMonitor->registered());
}

TEST_F(TDRegionMonitor, registered)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    ASSERT_FALSE(regionMonitor->registered());
}

TEST_F(TDRegionMonitor, watchedRegion)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    ASSERT_EQ(regionMonitor->watchedRegion(), QRegion());
}

TEST_F(TDRegionMonitor, coordinateType)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    ASSERT_EQ(regionMonitor->coordinateType(), DRegionMonitor::ScaleRatio);
}

TEST_F(TDRegionMonitor, registerRegionArg)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    ASSERT_EQ(regionMonitor->watchedRegion(), QRegion());
    QRegion r(0, 0, 600, 400);
    regionMonitor->registerRegion(r);
    ASSERT_EQ(regionMonitor->watchedRegion(), r);
}

TEST_F(TDRegionMonitor, unregisterRegion)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    regionMonitor->registerRegion();
    ASSERT_TRUE(regionMonitor->registered());
    regionMonitor->unregisterRegion();
    ASSERT_FALSE(regionMonitor->registered());
}

TEST_F(TDRegionMonitor, setWatchedRegion)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    ASSERT_EQ(regionMonitor->watchedRegion(), QRegion());
    QRegion r(0, 0, 600, 400);
    regionMonitor->setWatchedRegion(r);
    ASSERT_EQ(regionMonitor->watchedRegion(), r);
}

TEST_F(TDRegionMonitor, setCoordinateType)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    ASSERT_EQ(regionMonitor->coordinateType(), DRegionMonitor::ScaleRatio);
    regionMonitor->setCoordinateType(DRegionMonitor::Original);
    ASSERT_EQ(regionMonitor->coordinateType(), DRegionMonitor::Original);
}

TEST_F(TDRegionMonitor, registerFlags)
{
#ifndef QT_NO_DEBUG_STREAM
    enum { TestRegionFlag = (DRegionMonitor::Motion | DRegionMonitor::Button) };

    regionMonitor->setRegisterFlags(DRegionMonitor::RegisterdFlag(TestRegionFlag));
    ASSERT_EQ(regionMonitor->registerFlags(), TestRegionFlag);
#endif
}

TEST_F(TDRegionMonitor, privateFunctions)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    DRegionMonitorPrivate *region_d = regionMonitor->d_func();
    ASSERT_TRUE(region_d);

    regionMonitor->registerRegion();
    ASSERT_TRUE(region_d->registered());

    region_d->registerMonitorRegion();
    ASSERT_FALSE(region_d->registerKey.isEmpty());

    region_d->watchedRegion = {0, 0, 30, 30};
    region_d->registerMonitorRegion();
    ASSERT_FALSE(region_d->registerKey.isEmpty());

    region_d->unregisterMonitorRegion();
    ASSERT_TRUE(region_d->registerKey.isEmpty());

    enum { TestFlag = 1 | 2, TestPos = 20 };
    const QString testKey = "Key_Enter";

    QSignalSpy btnPressSpy(regionMonitor, SIGNAL(buttonPress(const QPoint &, const int)));
    region_d->_q_ButtonPress(TestFlag, TestPos, TestPos, region_d->registerKey);
    ASSERT_EQ(btnPressSpy.count(), 1);
    auto pressArguments = btnPressSpy.takeFirst();
    ASSERT_EQ(pressArguments.at(0).toPoint(), region_d->deviceScaledCoordinate({TestPos, TestPos}, qApp->devicePixelRatio()));
    ASSERT_EQ(pressArguments.at(1).toInt(), TestFlag);

    QSignalSpy btnreleaseSpy(regionMonitor, SIGNAL(buttonRelease(const QPoint &, const int)));
    region_d->_q_ButtonRelease(TestFlag, TestPos, TestPos, region_d->registerKey);
    ASSERT_EQ(btnreleaseSpy.count(), 1);
    auto releaseArguments = btnreleaseSpy.takeFirst();
    ASSERT_EQ(releaseArguments.at(0).toPoint(), region_d->deviceScaledCoordinate({TestPos, TestPos}, qApp->devicePixelRatio()));
    ASSERT_EQ(releaseArguments.at(1).toInt(), TestFlag);

    QSignalSpy cursorMoveSpy(regionMonitor, SIGNAL(cursorMove(const QPoint &)));
    region_d->_q_CursorMove(TestPos, TestPos, region_d->registerKey);
    ASSERT_EQ(cursorMoveSpy.count(), 1);
    auto curMoveArguments = cursorMoveSpy.takeFirst();
    ASSERT_EQ(curMoveArguments.at(0).toPoint(), region_d->deviceScaledCoordinate({TestPos, TestPos}, qApp->devicePixelRatio()));

    QSignalSpy cursorEnterSpy(regionMonitor, SIGNAL(cursorEnter(const QPoint &)));
    region_d->_q_CursorEnter(TestPos, TestPos, region_d->registerKey);
    ASSERT_EQ(cursorEnterSpy.count(), 1);
    auto curEnterArguments = cursorEnterSpy.takeFirst();
    ASSERT_EQ(curEnterArguments.at(0).toPoint(), region_d->deviceScaledCoordinate({TestPos, TestPos}, qApp->devicePixelRatio()));

    QSignalSpy cursorLeaveSpy(regionMonitor, SIGNAL(cursorLeave(const QPoint &)));
    region_d->_q_CursorLeave(TestPos, TestPos, region_d->registerKey);
    ASSERT_EQ(cursorLeaveSpy.count(), 1);
    auto curLeaveArguments = cursorLeaveSpy.takeFirst();
    ASSERT_EQ(curLeaveArguments.at(0).toPoint(), region_d->deviceScaledCoordinate({TestPos, TestPos}, qApp->devicePixelRatio()));

    QSignalSpy keyPressSpy(regionMonitor, SIGNAL(keyPress(const QString &)));
    region_d->_q_KeyPress(testKey, TestPos, TestPos, region_d->registerKey);
    ASSERT_EQ(keyPressSpy.count(), 1);
    auto keyPressArguments = keyPressSpy.takeFirst();
    ASSERT_EQ(keyPressArguments.at(0).toString(), testKey);

    QSignalSpy keyReleaseSpy(regionMonitor, SIGNAL(keyRelease(const QString &)));
    region_d->_q_KeyRelease(testKey, TestPos, TestPos, region_d->registerKey);
    ASSERT_EQ(keyReleaseSpy.count(), 1);
    auto keyReleaseArguments = keyReleaseSpy.takeFirst();
    ASSERT_EQ(keyReleaseArguments.at(0).toString(), testKey);
}

DGUI_END_NAMESPACE
