find_package(Qt5 REQUIRED COMPONENTS X11Extras)
pkg_check_modules(XCB IMPORTED_TARGET xcb)

set(BIN_NAME frameless-window)

add_executable(${BIN_NAME}
    main.cpp
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt5::Gui
    Qt5::GuiPrivate
    Qt5::X11Extras
    PkgConfig::XCB
)
