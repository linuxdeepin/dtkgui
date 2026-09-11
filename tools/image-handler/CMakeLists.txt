set(BIN image-handler)
set(TARGET_NAME ${BIN}${DTK_NAME_SUFFIX})

add_executable(${TARGET_NAME}
    main.cpp
)

# DImageHandler is deprecated but still used by this tool,
# so ignore deprecation warnings when building it.
target_compile_definitions(${TARGET_NAME} PRIVATE D_IGNORE_DEPRECATIONS)

target_link_libraries(${TARGET_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    ${LIB_NAME}
)
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${BIN})
