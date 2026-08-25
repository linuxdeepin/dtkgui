// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

// 演示场景：
// 1. 实例 A 启动，成为单实例 primary
// 2. 3 秒后 A 触发退出：aboutToQuit 发出（dtkgui 释放单实例锁和 server），
//    但 main() 中模拟"有线程卡住"，进程继续存活 10 秒才真正退出
// 3. 在这 10 秒内启动实例 B：
//    - 若在 aboutToQuit 之前启动 → B 检测到 A 存活，正常退出（单实例保护）
//    - 若在 aboutToQuit 之后启动 → B 应能成功成为新的 primary（本次修复的目标）

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!DGuiApplicationHelper::setSingleInstance("single-instance-demo")) {
        qInfo() << "[demo] another instance is running, exit now. pid =" << app.applicationPid();
        return 0;
    }

    qInfo() << "[demo] primary started. pid =" << app.applicationPid();

    // 可通过参数控制退出前的存活时间，默认 3 秒后自动退出
    int lifetime = 3000;
    if (app.arguments().contains("--lifetime")) {
        int pos = app.arguments().indexOf("--lifetime");
        if (pos + 1 < app.arguments().size())
            lifetime = app.arguments().at(pos + 1).toInt();
    }

    // 模拟退出阶段有线程卡住：事件循环退出后进程并不会立即结束
    const int stuckSeconds = 10;
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, [stuckSeconds] {
        qInfo() << "[demo] aboutToQuit: single instance released,"
                << "process will stay alive for" << stuckSeconds << "s (simulating stuck thread)";
    });

    QTimer::singleShot(lifetime, &app, &QCoreApplication::quit);
    const int ret = app.exec();

    // 模拟卡住的清理逻辑阻塞进程完全退出
    QThread::sleep(stuckSeconds);
    qInfo() << "[demo] stuck cleanup finished, real exit now.";

    return ret;
}
