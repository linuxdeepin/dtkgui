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
#define private public
#include "dplatformtheme.h"
#undef private
#include "dplatformtheme_p.h"

DGUI_USE_NAMESPACE

class TDPlatformTheme : public testing::Test
{
public:
    enum { TEST_COLOR = Qt::blue,
           TEST_DATA = 50 };
protected:
    void SetUp();
    void TearDown();

    QWindow *window;
    DPlatformTheme *theme;
    DPlatformThemePrivate *theme_d;

};

void TDPlatformTheme::SetUp()
{
    window = new QWindow;
    window->create();

    theme = new DPlatformTheme(quint32(window->winId()), window);
    theme_d = theme->d_func();
}

void TDPlatformTheme::TearDown()
{
    delete window;
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

#define ASSERT_EQ_BY_VALUE(SetFunc, Func, Param) \
    theme->SetFunc(Param); \
    ASSERT_EQ(theme->Func(), Param);


#define ASSERT_EQ_BY_NAME(SetFunc, Func, Param) \
    theme->SetFunc(TEST_THEME_NAME(Param)); \
    ASSERT_EQ(theme->Func(), TEST_THEME_NAME(Param));


#define UT_TEST_F(className, setFuncName, getFuncName, Param) \
    TEST_F(className, setFuncName) {\
    if (!qgetenv("QT_QPA_PLATFORM").contains("offscreen")) \
        ASSERT_EQ_BY_VALUE(setFuncName, getFuncName, Param);\
}

#define UT_TEST_F2(className, setFuncName, getFuncName, Param) \
    TEST_F(className, setFuncName) {\
    if (!qgetenv("QT_QPA_PLATFORM").contains("offscreen")) \
        ASSERT_EQ_BY_NAME(setFuncName, getFuncName, Param);\
}


UT_TEST_F(TDPlatformTheme, setCursorBlinkTime, cursorBlinkTime, TEST_DATA);
UT_TEST_F(TDPlatformTheme, setCursorBlinkTimeout, cursorBlinkTimeout, TEST_DATA);
UT_TEST_F(TDPlatformTheme, setCursorBlink, cursorBlink, true)
UT_TEST_F(TDPlatformTheme, setDoubleClickDistance, doubleClickDistance, TEST_DATA);
UT_TEST_F(TDPlatformTheme, setDoubleClickTime, doubleClickTime, TEST_DATA)
UT_TEST_F(TDPlatformTheme, setDndDragThreshold, dndDragThreshold, TEST_DATA);
UT_TEST_F2(TDPlatformTheme, setThemeName, themeName, "Theme");
UT_TEST_F2(TDPlatformTheme, setIconThemeName, iconThemeName, "Icon");
UT_TEST_F2(TDPlatformTheme, setSoundThemeName, soundThemeName, "Sound");
UT_TEST_F2(TDPlatformTheme, setFontName, fontName, "Font");
UT_TEST_F2(TDPlatformTheme, setMonoFontName, monoFontName, "MonoFont");
UT_TEST_F2(TDPlatformTheme, setGtkFontName, gtkFontName, "GtkFont");
UT_TEST_F(TDPlatformTheme, setFontPointSize, fontPointSize, TEST_DATA);
UT_TEST_F(TDPlatformTheme, setActiveColor, activeColor, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setWindow, window, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setWindowText, windowText, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setBase, base, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setAlternateBase, alternateBase, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setToolTipBase, toolTipBase, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setToolTipText, toolTipText, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setText, text, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setButton, button, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setButtonText, buttonText, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setBrightText, brightText, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setLight, light, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setMidlight, midlight, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setDark, dark, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setMid, mid, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setShadow, shadow, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setHighlight, highlight, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setLink, link, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setLinkVisited, linkVisited, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setItemBackground, itemBackground, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setTextTitle, textTitle, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setTextTips, textTips, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setTextWarning, textWarning, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setTextLively, textLively, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setLightLively, lightLively, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setDarkLively, darkLively, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setFrameBorder, frameBorder, TEST_COLOR);
UT_TEST_F(TDPlatformTheme, setWindowRadius, windowRadius, TEST_DATA / 10);
