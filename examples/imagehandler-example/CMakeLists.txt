set(BIN imagehandler)

add_executable(${BIN}
    main.cpp
)

target_link_libraries(${BIN} PRIVATE
    Qt5::Gui
    ${LIB_NAME}
    Dtk::Core
)
