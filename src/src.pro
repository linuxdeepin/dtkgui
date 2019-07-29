TARGET = dtkgui
TEMPLATE = lib
QT += dtkcore gui gui-private
load(dtk_build)

CONFIG += internal_module

load(dtk_cmake)
load(dtk_module)

include(private/private.pri)

HEADERS += \
    dnativesettings.h \
    dtkgui_global.h

SOURCES += \
    dnativesettings.cpp
