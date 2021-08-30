/*
 * Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     Wang Peng <993381@qq.com>
 *
 * Maintainer: Wang Peng <wangpenga@uniontech.com>
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
#include "DGuiApplicationHelper"
#include "test.h"
#include <QDebug>
#include <QWindow>

DGUI_USE_NAMESPACE

class TDGuiApplicationHelper : public DTest
{
protected:
    void SetUp();
    void TearDown();
};

void TDGuiApplicationHelper::SetUp()
{
}

void TDGuiApplicationHelper::TearDown()
{
}

TEST_F(TDGuiApplicationHelper, testLoad)
{
    QWindow window;
    DGuiApplicationHelper::instance()->windowTheme(&window);

    QColor color1(0, 0, 0, 0);
    QColor color2(DGuiApplicationHelper::instance()->adjustColor(color1, 0, 0, 0, 0, 0, 0, 0));
    ASSERT_EQ(color1, color2);

    QColor color3(DGuiApplicationHelper::blendColor(color1, color2));
    ASSERT_EQ(color1, color3);

    DGuiApplicationHelper::instance()->setApplicationPalette(QPalette());
    DGuiApplicationHelper::instance()->toColorType(QPalette());
    DGuiApplicationHelper::instance()->toColorType(QColor());

    // test theme type
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
    ASSERT_EQ(DGuiApplicationHelper::instance()->themeType(), DGuiApplicationHelper::DarkType);

    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    ASSERT_EQ(DGuiApplicationHelper::instance()->themeType(), DGuiApplicationHelper::LightType);

    // test palette type
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::ColorType::DarkType);
    ASSERT_EQ(DGuiApplicationHelper::instance()->paletteType(), DGuiApplicationHelper::ColorType::DarkType);

    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::ColorType::LightType);
    ASSERT_EQ(DGuiApplicationHelper::instance()->paletteType(), DGuiApplicationHelper::ColorType::LightType);

    DGuiApplicationHelper::instance()->setSingelInstanceInterval(300);

    QPalette palette = DGuiApplicationHelper::instance()->applicationPalette();

    DGuiApplicationHelper::instance()->setUseInactiveColorGroup(true);

    // test generate palette
    DPalette basePalette;
    QPalette::ColorRole role =  QPalette::ColorRole::Dark;
    basePalette.setColor(DPalette::Active, role, QColor(Qt::red));
    DGuiApplicationHelper::ColorType type = DGuiApplicationHelper::ColorType::DarkType;
    DGuiApplicationHelper::instance()->generatePalette(basePalette, type);

    //  test generate color role
    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);
        type = DGuiApplicationHelper::ColorType::DarkType;
        DGuiApplicationHelper::instance()->generatePaletteColor(basePalette, role, type);

        type = DGuiApplicationHelper::ColorType::LightType;
        DGuiApplicationHelper::instance()->generatePaletteColor(basePalette, role, type);

        type = DGuiApplicationHelper::ColorType::UnknownType;
        DGuiApplicationHelper::instance()->generatePaletteColor(basePalette, role, type);
    }

    DPalette dPallette1 = DGuiApplicationHelper::instance()->standardPalette(DGuiApplicationHelper::ColorType::LightType);
    DPalette dPallette2 = DGuiApplicationHelper::instance()->standardPalette(DGuiApplicationHelper::ColorType::DarkType);
    DPalette dPallette3 = DGuiApplicationHelper::instance()->standardPalette(DGuiApplicationHelper::ColorType::UnknownType);

    ASSERT_NE(dPallette1, dPallette2);
    ASSERT_NE(dPallette2, dPallette3);
}
