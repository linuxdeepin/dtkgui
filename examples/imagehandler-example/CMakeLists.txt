set(CMAKE_AUTOMOC ON)
find_package(DtkCore REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Gui)

set(BIN imagehandler)

add_executable(${BIN}
    main.cpp
)

target_link_libraries(
    ${BIN} PRIVATE
    Qt5::Gui
    dtkgui
    ${DtkCore_LIBRARIES}
)