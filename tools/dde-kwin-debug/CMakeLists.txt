set(BIN dde-kwin-bug)
find_package(PkgConfig REQUIRED)
pkg_check_modules(x11 REQUIRED x11)

add_executable(${BIN}
  main.cpp 
)
target_link_libraries(${BIN} PRIVATE 
  ${x11_LIBRARIES}
  -lpthread
)
target_include_directories(${BIN} PUBLIC
  ${x11_INCLUDE_DIRS}
)
install(TARGETS ${BIN} DESTINATION "${CMAKE_INSTALL_LIBDIR}/libdtk-${PROJECT_VERSION}/DGui/bin/")
