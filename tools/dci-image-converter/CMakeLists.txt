set(BIN_NAME dci-image-converter)

add_executable(${BIN_NAME}
    main.cpp
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
)
