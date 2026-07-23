file(GLOB KERNEL_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/../../include/kernel/*.h
)
file(GLOB KERNEL_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
set(kernel_SRC 
  ${KERNEL_HEADER}
  ${KERNEL_SOURCE}
)

if(LINUX)
  set(_PREF_CONFIG_JSON /usr/share/dsg/configs/org.deepin.dtk.preference.json)
else()
  set(_PREF_CONFIG_JSON ${CMAKE_INSTALL_PREFIX}/share/dsg/configs/org.deepin.dtk.preference.json)
endif()

dtk_add_config_to_cpp(kernel_SRC ${_PREF_CONFIG_JSON}
  OUTPUT_FILE_NAME orgdeepindtkpreference.hpp
  CLASS_NAME OrgDeepinDTKPreference
)
