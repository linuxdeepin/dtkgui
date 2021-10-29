isEmpty(DTK_DISABLE_LIBXDG):linux {
    CONFIG += link_pkgconfig
    PKGCONFIG += Qt5Xdg
    PKG_CONFIG = $$pkgConfigExecutable()
    XDG_ICON_VERSION = $$system($$PKG_CONFIG --modversion Qt5XdgIconLoader)
    XDG_ICON_VERSION_LIST = $$split(XDG_ICON_VERSION, .)
    INCLUDEPATH += $$system($$PKG_CONFIG --variable includedir Qt5XdgIconLoader)/qt5xdgiconloader/$$XDG_ICON_VERSION
    DEFINES += "XDG_ICON_VERSION_MAR=$$first(XDG_ICON_VERSION_LIST)"

    HEADERS += $$PWD/private/xdgiconproxyengine_p.h
    SOURCES += $$PWD/private/xdgiconproxyengine.cpp
} else {
    DEFINES += DTK_DISABLE_LIBXDG
}

HEADERS += \
    $$PWD/dfontmanager.h \
    $$PWD/dsvgrenderer.h \
    $$PWD/dtaskbarcontrol.h \
    $$PWD/dthumbnailprovider.h \
    $$PWD/dicontheme.h \
    $$PWD/private/dbuiltiniconengine_p.h

SOURCES += \
    $$PWD/dfontmanager.cpp \
    $$PWD/dsvgrenderer.cpp \
    $$PWD/dtaskbarcontrol.cpp \
    $$PWD/dthumbnailprovider.cpp \
    $$PWD/dicontheme.cpp \
    $$PWD/private/dbuiltiniconengine.cpp

RESOURCES += $$PWD/icons/deepin-theme-plugin-icons.qrc
