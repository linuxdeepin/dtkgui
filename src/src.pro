TARGET = dtkgui
TEMPLATE = lib
QT += dtkcore gui gui-private
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
    $$PWD/dplatformtheme.h

SOURCES += \
    $$PWD/dnativesettings.cpp \
    $$PWD/dwindowmanagerhelper.cpp \
    $$PWD/dforeignwindow.cpp \
    $$PWD/dwindowgroupleader.cpp \
    $$PWD/dplatformhandle.cpp \
    $$PWD/dpalette.cpp \
    $$PWD/dguiapplicationhelper.cpp \
    $$PWD/dplatformtheme.cpp

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
    $$PWD/DPlatformTheme

DTK_MODULE_NAME=$$TARGET
load(dtk_build)

INSTALLS += includes target

load(dtk_cmake)
load(dtk_module)
