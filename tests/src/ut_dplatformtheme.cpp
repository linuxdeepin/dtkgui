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
#include <gtest/gtest.h>
#include <QWindow>
#include <QTest>

#include "dguiapplicationhelper.h"
#include "dplatformtheme.h"
#include "dplatformtheme_p.h"

DGUI_USE_NAMESPACE

class TDPlatformTheme : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    QWidget *widget;
    DPlatformTheme *theme;
    DPlatformThemePrivate *theme_d;
};

void TDPlatformTheme::SetUp()
{
    widget = new QWidget;
    widget->show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(widget));

    theme = new DPlatformTheme(widget->windowHandle()->winId(), widget);
    theme_d = theme->d_func();
}

void TDPlatformTheme::TearDown()
{
    delete widget;
}

TEST_F(TDPlatformTheme, testFunction)
{
    ASSERT_TRUE(theme_d->theme);
    ASSERT_EQ(theme->parentTheme(), theme_d->parent);
    ASSERT_FALSE(theme->isValid());

    DPalette tPalette = theme->palette();
    ASSERT_EQ(theme->fetchPalette(tPalette), tPalette);
}

#define TEST_THEME_NAME(TYPE) \
    QByteArrayLiteral("Test_Name_About_").append(TYPE)

#define ASSERT_EQ_BY_VALUE(SetFunc,Func,Param) \
    theme->SetFunc(Param); \
    ASSERT_EQ(theme->Func(),Param); \

#define ASSERT_EQ_COLOR(SetFunc,Func,Param) \
    ASSERT_EQ_BY_VALUE(SetFunc,Func,Param); \

#define ASSERT_EQ_BY_NAME(SetFunc,Func,Param) \
    theme->SetFunc(TEST_THEME_NAME(Param)); \
    ASSERT_EQ(theme->Func(),TEST_THEME_NAME(Param));

TEST_F(TDPlatformTheme, testSetFunction)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    if (!theme->isValid()) {
        qDebug() << "PlatformTheme is not valid";
        return;
    }

    enum { TEST_COLOR = Qt::blue, TEST_DATA = 50 };

    ASSERT_EQ_BY_VALUE(setCursorBlinkTime, cursorBlinkTime, TEST_DATA);
    ASSERT_EQ_BY_VALUE(setCursorBlinkTimeout, cursorBlinkTimeout, TEST_DATA);
    ASSERT_EQ_BY_VALUE(setCursorBlink, cursorBlink, true)
    ASSERT_EQ_BY_VALUE(setDoubleClickDistance, doubleClickDistance, TEST_DATA);
    ASSERT_EQ_BY_VALUE(setDoubleClickTime, doubleClickTime, TEST_DATA)
    ASSERT_EQ_BY_VALUE(setDndDragThreshold, dndDragThreshold, TEST_DATA);
    ASSERT_EQ_BY_NAME(setThemeName, themeName, "Theme");
    ASSERT_EQ_BY_NAME(setIconThemeName, iconThemeName, "Icon");
    ASSERT_EQ_BY_NAME(setSoundThemeName, soundThemeName, "Sound");
    ASSERT_EQ_BY_NAME(setFontName, fontName, "Font");
    ASSERT_EQ_BY_NAME(setMonoFontName, monoFontName, "MonoFont");
    ASSERT_EQ_BY_NAME(setGtkFontName, gtkFontName, "GtkFont");
    ASSERT_EQ_BY_VALUE(setFontPointSize, fontPointSize, TEST_DATA);
    ASSERT_EQ_BY_VALUE(setActiveColor, activeColor, TEST_COLOR);
    ASSERT_EQ_COLOR(setWindow, window, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setWindowText, windowText, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setBase, base, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setAlternateBase, alternateBase, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setToolTipBase, toolTipBase, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setToolTipText, toolTipText, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setText, text, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setButton, button, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setButtonText, buttonText, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setBrightText, brightText, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setLight, light, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setMidlight, midlight, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setDark, dark, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setMid, mid, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setShadow, shadow, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setHighlight, highlight, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setLink, link, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setLinkVisited, linkVisited, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setItemBackground, itemBackground, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setTextTitle, textTitle, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setTextTips, textTips, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setTextWarning, textWarning, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setTextLively, textLively, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setLightLively, lightLively, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setDarkLively, darkLively, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setFrameBorder, frameBorder, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setWindowRadius, windowRadius, TEST_DATA / 10);
}
