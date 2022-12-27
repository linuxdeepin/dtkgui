find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(BIN taskbar)

add_executable(${BIN}
    testtaskbarwindow.h 
    testtaskbarwindow.cpp
    main.cpp
)

target_link_libraries(${BIN} PRIVATE
    Qt5::Widgets
    Qt5::Gui
    ${LIB_NAME}
    Dtk::Core
)
