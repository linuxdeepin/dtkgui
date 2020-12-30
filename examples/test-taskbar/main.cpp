#include <QApplication>
#include <QDesktopWidget>

#include "testtaskbarwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto name = QUuid::createUuid().toString() + QStringLiteral(".desktop");
    a.setDesktopFileName(name);
    QDir appDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QFile desktopFile(appDir.absoluteFilePath(name));
    if(!desktopFile.exists()) {
        desktopFile.open(QIODevice::WriteOnly);
        desktopFile.write("[Desktop Entry]\n");
        desktopFile.write("Type=Application\n");
        desktopFile.write("Version=1.1\n");
        desktopFile.write("Name=" + QApplication::applicationDisplayName().toUtf8() + "\n");
        desktopFile.write("Exec=" + QApplication::applicationFilePath().toUtf8() + "\n");
        desktopFile.close();
    }

    TestTaskbarWindow *pTaskbarWindow = new TestTaskbarWindow;
    pTaskbarWindow->showMaximized();

    //这里删除用于测试的desktop文件
    QObject::connect(pTaskbarWindow, &TestTaskbarWindow::closeWindow, [&desktopFile] {
        QFile::remove(desktopFile.fileName());
    });

    return a.exec();
}
