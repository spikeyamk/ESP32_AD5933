if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/temperature.cpp"
                       	   INCLUDE_DIRS "include"
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(AD5933_debug_data C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(AD5933_REG_TYPES_DIR ${CMAKE_CURRENT_LIST_DIR}/../reg_types)
	set(AD5933_TEMPERATURE_DIR ${CMAKE_CURRENT_LIST_DIR}/../temperature)
	set(AD5933_DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/../data)
	set(AD5933_MASKS_DIR ${CMAKE_CURRENT_LIST_DIR}/../masks)

	if(NOT TARGET AD5933_reg_types)
		add_subdirectory(${AD5933_REG_TYPES_DIR} {CMAKE_BINARY_DIR}/AD5933_reg_types EXCLUDE_FROM_ALL)
	endif()
	if(NOT TARGET AD5933_temperature)
		add_subdirectory(${AD5933_TEMPERATURE_DIR} {CMAKE_BINARY_DIR}/AD5933_temperature EXCLUDE_FROM_ALL)
	endif()
	if(NOT TARGET AD5933_data)
		add_subdirectory(${AD5933_DATA_DIR} {CMAKE_BINARY_DIR}/AD5933_data EXCLUDE_FROM_ALL)
	endif()
	if(NOT TARGET AD5933_masks)
		add_subdirectory(${AD5933_MASKS_DIR} {CMAKE_BINARY_DIR}/AD5933_masks EXCLUDE_FROM_ALL)
	endif()

	set(AD5933_DEBUG_DATA_SRCS
		lib/debug_data.cpp
	)

	add_library(AD5933_debug_data STATIC
		${AD5933_DEBUG_DATA_SRCS}
	)

	target_include_directories(AD5933_debug_data PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)

	target_link_libraries(AD5933_debug_data PRIVATE
		AD5933_reg_types
		AD5933_temperature
		AD5933_data
		AD5933_masks
	)
endif()
