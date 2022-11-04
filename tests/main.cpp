// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"

#include <QApplication>
#include <gtest/gtest.h>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    // gerrit编译时没有显示器，需要指定环境变量
    if (!qEnvironmentVariableIsSet("DISPLAY"))
        qputenv("QT_QPA_PLATFORM", "offscreen");

    // for ut_dicon.cpp 测试指定缩放时 dicon 的 ut
    // 如果需要删除，请一起删除 ut_dicon.cpp 中 ASSERT_FLOAT_EQ(devicePixelRatio, 1.25);
    qputenv("D_DXCB_DISABLE_OVERRIDE_HIDPI","1");
    qputenv("QT_SCALE_FACTOR","1.25");

    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan.log");
#endif

    return ret;
}
