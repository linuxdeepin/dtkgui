TEMPLATE = app
QT += dtkcore
CONFIG -= app_bundle
CONFIG += thread

CONFIG += testcase no_testcase_installs
# debug 才生成你这些信息
CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
    QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0
}

INCLUDEPATH += $$PWD/../src
DEPENDPATH += $$PWD/../src
unix:QMAKE_RPATHDIR += $$OUT_PWD/../src
unix:LIBS += -L$$OUT_PWD/../src/ -ldtkgui -lgtest

HEADERS += \
    test.h

SOURCES += \
        main.cpp \
    src/tdregionmonitor.cpp \
    src/tdforeignwindow.cpp \
    src/tdguiapplicationhelper.cpp
