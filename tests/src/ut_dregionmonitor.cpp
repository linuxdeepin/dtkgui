#include <gtest/gtest.h>
#include "dregionmonitor.h"

#include <QRegion>

DGUI_BEGIN_NAMESPACE

class TDRegionMonitor : public testing::Test
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
    //    regionMonitor->registerRegion();
    //    ASSERT_TRUE(regionMonitor->registered());
}

TEST_F(TDRegionMonitor, registered)
{
    ASSERT_FALSE(regionMonitor->registered());
}

TEST_F(TDRegionMonitor, watchedRegion)
{
    ASSERT_EQ(regionMonitor->watchedRegion(), QRegion());
}

TEST_F(TDRegionMonitor, coordinateType)
{
    ASSERT_EQ(regionMonitor->coordinateType(), DRegionMonitor::ScaleRatio);
}

TEST_F(TDRegionMonitor, registerRegionArg)
{
    ASSERT_EQ(regionMonitor->watchedRegion(), QRegion());
    QRegion r(0, 0, 600, 400);
    regionMonitor->registerRegion(r);
    ASSERT_EQ(regionMonitor->watchedRegion(), r);
}

TEST_F(TDRegionMonitor, unregisterRegion)
{
    //    regionMonitor->registerRegion();
    //    ASSERT_TRUE(regionMonitor->registered());
    //    regionMonitor->unregisterRegion();
    //    ASSERT_FALSE(regionMonitor->registered());
}

TEST_F(TDRegionMonitor, setWatchedRegion)
{
    ASSERT_EQ(regionMonitor->watchedRegion(), QRegion());
    QRegion r(0, 0, 600, 400);
    regionMonitor->setWatchedRegion(r);
    ASSERT_EQ(regionMonitor->watchedRegion(), r);
}

TEST_F(TDRegionMonitor, setCoordinateType)
{
    ASSERT_EQ(regionMonitor->coordinateType(), DRegionMonitor::ScaleRatio);
    regionMonitor->setCoordinateType(DRegionMonitor::Original);
    ASSERT_EQ(regionMonitor->coordinateType(), DRegionMonitor::Original);
}

DGUI_END_NAMESPACE
