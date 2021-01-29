#include <gtest/gtest.h>
#include "dguiapplicationhelper.h"

#include <QMap>

DGUI_BEGIN_NAMESPACE

class TDGuiApplicationHelper : public testing::Test
{
protected:
    virtual void SetUp()
    {
        helper = DGuiApplicationHelper::instance();

        /* read write */
        readWriteDatas << DGuiApplicationHelper::Attribute::UseInactiveColorGroup
                       << DGuiApplicationHelper::Attribute::ColorCompositing;

        /* read only */
        readOnlyDatas << DGuiApplicationHelper::Attribute::IsDeepinPlatformTheme
                      << DGuiApplicationHelper::Attribute::IsDXcbPlatform
                      << DGuiApplicationHelper::Attribute::IsXWindowPlatform
                      << DGuiApplicationHelper::Attribute::IsTableEnvironment
                      << DGuiApplicationHelper::Attribute::IsDeepinEnvironment;
    }

    DGuiApplicationHelper *helper = nullptr;
    QList<DGuiApplicationHelper::Attribute> readWriteDatas;
    QList<DGuiApplicationHelper::Attribute> readOnlyDatas;
};

TEST_F(TDGuiApplicationHelper, AttributeReadWrite)
{
    QMap<DGuiApplicationHelper::Attribute, bool> oldData;

    for (const DGuiApplicationHelper::Attribute attribute : readWriteDatas) {
        oldData[attribute] = helper->testAttribute(attribute);
    }

    for (const DGuiApplicationHelper::Attribute attribute : readWriteDatas) {
        helper->setAttribute(attribute, !oldData[attribute]);
        EXPECT_EQ(helper->testAttribute(attribute), !oldData[attribute]);
    }
}

TEST_F(TDGuiApplicationHelper, AttributeReadOnly)
{
    QMap<DGuiApplicationHelper::Attribute, bool> oldData;

    for (const DGuiApplicationHelper::Attribute attribute : readOnlyDatas) {
        oldData[attribute] = helper->testAttribute(attribute);
    }

    for (const DGuiApplicationHelper::Attribute attribute : readOnlyDatas) {
        helper->setAttribute(attribute, !oldData[attribute]);
        EXPECT_EQ(helper->testAttribute(attribute), oldData[attribute]);
    }
}

DGUI_END_NAMESPACE
