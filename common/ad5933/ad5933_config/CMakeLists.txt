if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/config.cpp"
                       	   INCLUDE_DIRS "include"
						   REQUIRES ad5933_reg_types ad5933_uint_types ad5933_masks
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(AD5933_config C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(AD5933_CONFIG_SRCS
		lib/config.cpp
	)

	set(AD5933_REG_TYPES_DIR ${CMAKE_CURRENT_LIST_DIR}/../ad5933_reg_types)
	set(AD5933_UINT_TYPES_DIR ${CMAKE_CURRENT_LIST_DIR}/../ad5933_uint_types)
	set(AD5933_MASKS_DIR ${CMAKE_CURRENT_LIST_DIR}/../ad5933_masks)

	add_subdirectory(${AD5933_REG_TYPES_DIR} ${CMAKE_BINARY_DIR}/reg_types)
	add_subdirectory(${AD5933_UINT_TYPES_DIR} ${CMAKE_BINARY_DIR}/uint_types)
	add_subdirectory(${AD5933_MASKS_DIR} ${CMAKE_BINARY_DIR}/masks)

	set(AD5933_CONFIG_LINK_LIBS
		AD5933_reg_types
		AD5933_uint_types
		AD5933_masks
	)

	option(AD5933_CONFIG_TEST "Enable AD5933::Config Tests" OFF)
	if(AD5933_CONFIG_TEST)
		list(APPEND AD5933_CONFIG_SRCS "lib/tests.cpp")
		set(TEST_SRCS 
			lib/tests.cpp
		)

		function(add_test_case TEST_NAME TEST_SOURCE)
			add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
			target_include_directories(${TEST_NAME} PRIVATE
				${CMAKE_CURRENT_LIST_DIR}/include
				${AD5933_REG_TYPES_DIR}/include
			)
			target_link_libraries(${TEST_NAME} PRIVATE
				${AD5933_CONFIG_LINK_LIBS}
			)
			add_test(NAME AD5933::Config::Tests::${TEST_NAME}
					COMMAND ${TEST_NAME})
		endfunction()

		enable_testing()
		add_test_case(run_test_array run_test_array.cpp)
		add_test_case(run_test_same_setters run_test_same_setters.cpp)
		add_test_case(run_test_different_setters run_test_different_setters.cpp)
		add_test_case(run_test_to_array_to_config run_test_to_array_to_config.cpp)
	endif()

	add_library(AD5933_config STATIC
		${AD5933_CONFIG_SRCS}
	)

	target_include_directories(AD5933_config PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)

	target_link_libraries(AD5933_config PUBLIC
		${AD5933_CONFIG_LINK_LIBS}
	)
endif()
