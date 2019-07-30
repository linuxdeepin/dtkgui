TARGET = dtkgui
TEMPLATE = lib
QT += dtkcore gui gui-private
CONFIG += internal_module

include(private/private.pri)

HEADERS += \
    $$PWD/dnativesettings.h \
    $$PWD/dtkgui_global.h

SOURCES += \
    $$PWD/dnativesettings.cpp

includes.files += \
    $$PWD/*.h \
    $$PWD/DtkGuis \
    $$PWD/dtkgui_config.h \
    $$PWD/DNativeSettings

DTK_MODULE_NAME=$$TARGET
load(dtk_build)

INSTALLS += includes target

load(dtk_cmake)
load(dtk_module)
