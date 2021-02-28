TARGET = dtkgui
TEMPLATE = lib
QT += dtkcore gui gui-private dbus network
CONFIG += internal_module

# 龙芯架构上没有默认添加PT_GNU_STACK-section,所以此处手动指定一下
contains(QMAKE_HOST.arch, mips.*): QMAKE_LFLAGS_SHLIB += "-Wl,-z,noexecstack"

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
    $$PWD/dsvgrenderer.h \
    $$PWD/dthumbnailprovider.h \
    $$PWD/dtaskbarcontrol.h

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
    $$PWD/dfiledragclient.cpp \
    $$PWD/dthumbnailprovider.cpp \
    $$PWD/dtaskbarcontrol.cpp

includes.files += \
    $$PWD/*.h \
    $$PWD/DtkGuis \
    $$PWD/dtkgui_config.h \
    $$PWD/kernel/*.h \
    $$PWD/kernel/DForeignWindow \
    $$PWD/kernel/DGuiApplicationHelper \
    $$PWD/kernel/DNativeSettings \
    $$PWD/kernel/DPalette \
    $$PWD/kernel/DPlatformHandle \
    $$PWD/kernel/DPlatformTheme \
    $$PWD/kernel/DRegionMonitor \
    $$PWD/kernel/DWindowGroupLeader \
    $$PWD/kernel/DWindowManagerHelper \
    $$PWD/filedrag/*.h \
    $$PWD/filedrag/DFileDrag \
    $$PWD/filedrag/DFileDragClient \
    $$PWD/filedrag/DFileDragServer \
    $$PWD/util/*.h \
    $$PWD/util/DFontManager \
    $$PWD/util/DSvgRenderer \
    $$PWD/util/DTaskbarControl \
    $$PWD/util/DThumbnailProvider

dbus_monitor.files += $$PWD/dbus/com.deepin.api.XEventMonitor.xml
dbus_monitor.header_flags += -i dbus/arealist.h
DBUS_INTERFACES += dbus_monitor

DTK_MODULE_NAME=$$TARGET
load(dtk_build)

INSTALLS += includes target

load(dtk_cmake)
load(dtk_module)
