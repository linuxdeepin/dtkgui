TEMPLATE = app
QT += dtkcore

CONFIG += X11

SOURCES += \
    $$PWD/main.cpp

DESTDIR = $$_PRO_FILE_PWD_/../../bin

DTK_MODULE_NAME=dtkgui
load(dtk_build_config)
target.path = $$TOOL_INSTALL_DIR

INSTALLS += target
