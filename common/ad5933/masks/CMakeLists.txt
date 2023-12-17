if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/maps.cpp"
						   "lib/masks.cpp"
                       	   INCLUDE_DIRS "include"
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(AD5933_masks C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(AD5933_MASKS_SRCS
		lib/maps.cpp
		lib/masks.cpp
	)

	add_library(AD5933_masks STATIC
		${AD5933_MASKS_SRCS}
	)

	target_include_directories(AD5933_masks PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)
endif()
