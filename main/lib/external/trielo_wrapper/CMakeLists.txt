idf_component_register()
set(FMT_INSTALL OFF)
add_subdirectory(trielo)
target_link_libraries(${COMPONENT_LIB} INTERFACE trielo)
