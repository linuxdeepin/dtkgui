CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += \
    $$PWD/main.cpp \

# 支持从当前目录找到动态库
mac {
    QMAKE_RPATHDIR = "@executable_path"
}
