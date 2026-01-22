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

dtk_add_config_to_cpp(kernel_SRC /usr/share/dsg/configs/org.deepin.dtk.preference.json
  OUTPUT_FILE_NAME orgdeepindtkpreference.hpp
  CLASS_NAME OrgDeepinDTKPreference
)
