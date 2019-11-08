TARGET = dtkgui
TEMPLATE = lib
QT += dtkcore gui gui-private dbus network
CONFIG += internal_module

include(private/private.pri)

HEADERS += \
    $$PWD/dnativesettings.h \
    $$PWD/dtkgui_global.h \
    $$PWD/dwindowmanagerhelper.h \
    $$PWD/dforeignwindow.h \
    $$PWD/dwindowgroupleader.h \
    $$PWD/dplatformhandle.h \
    $$PWD/dpalette.h \
    $$PWD/dguiapplicationhelper.h \
    $$PWD/dplatformtheme.h \
    $$PWD/dfiledragserver.h \
    $$PWD/dfiledrag.h \
    $$PWD/dfiledragclient.h \

SOURCES += \
    $$PWD/dnativesettings.cpp \
    $$PWD/dwindowmanagerhelper.cpp \
    $$PWD/dforeignwindow.cpp \
    $$PWD/dwindowgroupleader.cpp \
    $$PWD/dplatformhandle.cpp \
    $$PWD/dpalette.cpp \
    $$PWD/dguiapplicationhelper.cpp \
    $$PWD/dplatformtheme.cpp \
    $$PWD/dfiledragserver.cpp \
    $$PWD/dfiledrag.cpp \
    $$PWD/dfiledragclient.cpp

includes.files += \
    $$PWD/*.h \
    $$PWD/DtkGuis \
    $$PWD/dtkgui_config.h \
    $$PWD/DNativeSettings \
    $$PWD/DWindowManagerHelper \
    $$PWD/DForeignWindow \
    $$PWD/DWindowGroupLeader \
    $$PWD/DPlatformHandle \
    $$PWD/DPalette \
    $$PWD/DGuiApplicationHelper \
    $$PWD/DPlatformTheme \
    $$PWD/DFileDragServer \
    $$PWD/DFileDrag \
    $$PWD/DFileDragClient

systembusconf.path = /etc/dbus-1/system.d
systembusconf.files = $$PWD/dbus/com.deepin.dtk.FileDrag.conf

DTK_MODULE_NAME=$$TARGET
load(dtk_build)

INSTALLS += includes target systembusconf

load(dtk_cmake)
load(dtk_module)
