find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(BIN single-instance-demo)
set(TARGET ${BIN}${DTK_VERSION_MAJOR})

add_executable(${TARGET}
    main.cpp
)

target_link_libraries(${TARGET} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${LIB_NAME}
)

set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${BIN})

install(TARGETS ${TARGET} DESTINATION "${TOOL_INSTALL_DIR}")
