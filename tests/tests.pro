TEMPLATE = app
QT += dtkcore gui gui-private dbus network testlib widgets
CONFIG += thread
CONFIG -= app_bundle

load(dtk_testcase)

INCLUDEPATH += \
    $$PWD/../src/ \
    $$PWD/../src/filedrag \
    $$PWD/../src/kernel \
    $$PWD/../src/util \
    $$OUT_PWD/../src \
    $$PWD/../src/private

QMAKE_CXXFLAGS += -fno-access-control
QMAKE_LFLAGS += -fno-access-control

# 指定moc文件生成目录和src一样
MOC_DIR=$$OUT_PWD/../src

DEPENDPATH += $$PWD/../src
unix:QMAKE_RPATHDIR += $$OUT_PWD/../src
unix:LIBS += -L$$OUT_PWD/../src/ -ldtkgui -lgtest -lglib-2.0

include($$PWD/../src/filedrag/filedrag.pri)
include($$PWD/../src/kernel/kernel.pri)
include($$PWD/../src/util/util.pri)

linux* {
    # don't link library
    QMAKE_CXXFLAGS += $$system(pkg-config --cflags-only-I librsvg-2.0)

    dbus_monitor.files += $$PWD/../src/dbus/com.deepin.api.XEventMonitor.xml
    dbus_monitor.header_flags += -i ../src/dbus/arealist.h
    DBUS_INTERFACES += dbus_monitor
}

HEADERS += \
    test.h

SOURCES += \
    main.cpp \
    src/ut_dguiapplicationhelper.cpp \
    src/ut_dregionmonitor.cpp \
    src/ut_dforeignwindow.cpp \
    src/ut_dpalette.cpp \
    src/ut_dplatformhandle.cpp \
    src/ut_dplatformtheme.cpp

