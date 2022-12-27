set(BIN_NAME dde-kwin-bug)
pkg_check_modules(X11 IMPORTED_TARGET REQUIRED x11)

add_executable(${BIN_NAME}
    main.cpp
)
target_link_libraries(${BIN_NAME} PRIVATE 
    PkgConfig::X11
    pthread
)

install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
