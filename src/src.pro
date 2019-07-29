TARGET = dtkgui
TEMPLATE = lib
QT += dtkcore gui gui-private
load(dtk_build)

CONFIG += internal_module

include(private/private.pri)

HEADERS += \
    dnativesettings.h \
    dtkgui_global.h

SOURCES += \
    dnativesettings.cpp

includes.files += $$PWD/*.h
includes.files += \
    $$PWD/DNativeSettings

# create DtkGuis file
defineTest(containIncludeFiles) {
    header = $$absolute_path($$ARGS)
    header_dir = $$quote($$dirname(header))

    for (file, includes.files) {
        file_ap = $$absolute_path($$file)
        file_dir = $$quote($$dirname(file_ap))

        isEqual(file_dir, $$header_dir):return(true)
    }

    return(false)
}

defineTest(updateDtkGuisFile) {
    dtkguis_include_files = $$HEADERS
    dtkguis_file_content = $$quote($${LITERAL_HASH}ifndef DTK_GUIS_MODULE_H)
    dtkguis_file_content += $$quote($${LITERAL_HASH}define DTK_GUIS_MODULE_H)

    for(header, dtkguis_include_files) {
        containIncludeFiles($$header) {
            dtkguis_file_content += $$quote($${LITERAL_HASH}include \"$$basename(header)\")
        }
    }

    dtkguis_file_content += $$quote($${LITERAL_HASH}endif)
    !write_file($$PWD/DtkGuis, dtkguis_file_content):return(false)

    return(true)
}

!updateDtkGuisFile():warning(Cannot create "DtkGuis" header file)

# create dtkgui_config.h file
defineTest(updateDtkGuiConfigFile) {
    for(file, includes.files) {
        file = $$quote($$basename(file))

        !isEqual(file, DtkGuis):contains(file, D[A-Za-z0-9_]+) {
            dtkgui_config_content += $$quote($${LITERAL_HASH}define DTKGUI_CLASS_$$file)
        }
    }

    !write_file($$PWD/dtkgui_config.h, dtkgui_config_content):return(false)

    return(true)
}

!updateDtkGuiConfigFile():warning(Cannot create "dtkgui_config.h" header file)

INSTALLS += includes target

load(dtk_cmake)
load(dtk_module)
