idf_component_register()
add_subdirectory(trielo)
target_link_libraries(${COMPONENT_LIB} INTERFACE trielo)
