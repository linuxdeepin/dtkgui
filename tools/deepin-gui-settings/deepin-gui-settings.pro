TEMPLATE = app
QT += dtkcore

SOURCES += \
    $$PWD/main.cpp

DESTDIR = $$_PRO_FILE_PWD_/../../bin

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/release -ldtkgui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/debug -ldtkgui
else:unix: LIBS += -L$$OUT_PWD/../../src -ldtkgui

INCLUDEPATH += $$PWD/../../src

CONFIG(debug, debug|release) {
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../../src
}
