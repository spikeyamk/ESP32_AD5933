if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/reg_addrs.cpp"
						   "lib/maps.cpp"
                       	   INCLUDE_DIRS "include"
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(AD5933::reg_addrs C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(AD5933_REG_ADDRS_SRCS
		lib/reg_addrs.cpp
		lib/maps.cpp
	)

	add_library(AD5933_reg_addrs STATIC
		${AD5933_REG_ADDRS_SRCS}
	)

	target_include_directories(AD5933_reg_addrs PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)
endif()
