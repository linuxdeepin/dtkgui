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
    $$PWD/dtaskbarcontrol.h \
    $$PWD/dfontmanager.h

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
    $$PWD/dtaskbarcontrol.cpp \
    $$PWD/dfontmanager.cpp

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
    $$PWD/DFileDragClient \
    $$PWD/DSvgRenderer \
    $$PWD/DThumbnailProvider \
    $$PWD/DTaskbarControl \
    $$PWD/DFontManager

linux* {
    # don't link library
    QMAKE_CXXFLAGS += $$system(pkg-config --cflags-only-I librsvg-2.0)

    HEADERS += \
        $$PWD/dregionmonitor.h

    SOURCES += \
        $$PWD/dregionmonitor.cpp \
        $$PWD/dsvgrenderer.cpp

    includes.files += \
        $$PWD/DRegionMonitor

    dbus_monitor.files += $$PWD/dbus/com.deepin.api.XEventMonitor.xml
    dbus_monitor.header_flags += -i dbus/arealist.h
    DBUS_INTERFACES += dbus_monitor
}

systembusconf.path = /etc/dbus-1/system.d
systembusconf.files = $$PWD/dbus/com.deepin.dtk.FileDrag.conf

DTK_MODULE_NAME=$$TARGET
load(dtk_build)

INSTALLS += includes target systembusconf

load(dtk_cmake)
load(dtk_module)
