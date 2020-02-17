QT += dtkcore gui widgets

TARGET = taskbar
TEMPLATE = app

CONFIG += c++11

SOURCES += \
        main.cpp \
        testtaskbarwindow.cpp

HEADERS += \
    testtaskbarwindow.h

DESTDIR = $$_PRO_FILE_PWD_/../../bin

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/release -ldtkgui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/debug -ldtkgui
else:unix: LIBS += -L$$OUT_PWD/../../src -ldtkgui

INCLUDEPATH += $$PWD/../../src

CONFIG(debug, debug|release) {
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../../src
}

DTK_MODULE_NAME=dtkgui
load(dtk_build_config)
target.path = $$TOOL_INSTALL_DIR

INSTALLS += target
