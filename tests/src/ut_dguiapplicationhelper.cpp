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
#include "dguiapplicationhelper.h"
#include "dguiapplicationhelper_p.h"

#include <QMap>

DGUI_BEGIN_NAMESPACE

class TDGuiApplicationHelper : public DTest
{
protected:
    virtual void SetUp()
    {
        helper = DGuiApplicationHelper::instance();
        helper_d = helper->d_func();

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
    DGuiApplicationHelperPrivate *helper_d = nullptr;
    QList<DGuiApplicationHelper::Attribute> readWriteDatas;
    QList<DGuiApplicationHelper::Attribute> readOnlyDatas;
};

TEST_F(TDGuiApplicationHelper, testFunction)
{
    QColor testColor(Qt::red);
    QColor adjustedColor = helper->adjustColor(testColor, 0, 0, 0, 0, 0, 0, -20);
    ASSERT_NE(testColor, adjustedColor);
    ASSERT_TRUE(adjustedColor.isValid());

    QColor disBlendColor  = Qt::black;
    QColor blendedColor = helper->blendColor(testColor, disBlendColor);
    ASSERT_NE(testColor, blendedColor);
    ASSERT_TRUE(blendedColor.isValid());

    DPalette tPalette;
    helper->generatePaletteColor(tPalette, QPalette::Window, DGuiApplicationHelper::ColorType::LightType);
    ASSERT_EQ(tPalette.brush(QPalette::Disabled, QPalette::Window), tPalette.brush(QPalette::Normal, QPalette::Window));

    tPalette.setColor(DPalette::Background, Qt::black);
    helper->generatePaletteColor(tPalette, QPalette::Highlight, DGuiApplicationHelper::ColorType::DarkType);
    ASSERT_TRUE(tPalette.highlight().color().isValid());

    // 初始化调色板为默认值
    helper->generatePalette(tPalette);

    ASSERT_EQ(helper->isXWindowPlatform(), DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsXWindowPlatform));
    ASSERT_EQ(helper->isTabletEnvironment(), DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsTableEnvironment));

    ASSERT_TRUE(helper->applicationTheme());
    ASSERT_TRUE(helper->systemTheme());
    ASSERT_NE(helper->applicationPalette(), tPalette);
    qGuiApp->setAttribute(Qt::AA_SetPalette, false);
    helper->setPaletteType(DGuiApplicationHelper::DarkType);
    ASSERT_NE(helper->applicationPalette(), tPalette);

    helper->setApplicationPalette(tPalette);
    ASSERT_EQ(helper->applicationPalette(), tPalette);

    ASSERT_TRUE(helper->fontManager());
    ASSERT_EQ(helper->toColorType(QColor(Qt::white)), DGuiApplicationHelper::LightType);
    ASSERT_EQ(helper->toColorType(QColor(Qt::black)), DGuiApplicationHelper::DarkType);
    ASSERT_EQ(helper->themeType(), DGuiApplicationHelper::DarkType);
    ASSERT_EQ(helper->paletteType(), DGuiApplicationHelper::DarkType);
}

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
