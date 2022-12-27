set(BIN image-handler)

add_executable(${BIN}
    main.cpp
)

target_link_libraries(${BIN} PRIVATE
    Qt5::Gui
    Dtk::Core
    ${LIB_NAME}
)
