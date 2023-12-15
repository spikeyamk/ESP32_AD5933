if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/config.cpp"
						   "lib/maps.cpp"
						   "lib/masks.cpp"
                       	   INCLUDE_DIRS "include"
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(ad5933_config C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(AD5933_CONFIG_SRCS
		lib/config.cpp
		lib/maps.cpp
		lib/masks.cpp
	)

	option(AD5933_CONFIG_TEST "Enable AD5933::Config Tests" OFF)
	if(AD5933_CONFIG_TEST)
		list(APPEND AD5933_CONFIG_SRCS "lib/tests.cpp")
		set(TEST_SRCS 
			lib/tests.cpp
		)

		function(add_test_case TEST_NAME TEST_SOURCE)
			add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
			target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
			add_test(NAME AD5933::Config::Tests::${TEST_NAME}
					COMMAND ${TEST_NAME})
		endfunction()

		enable_testing()
		add_test_case(run_test_array run_test_array.cpp)
		add_test_case(run_test_same_setters run_test_same_setters.cpp)
		add_test_case(run_test_different_setters run_test_different_setters.cpp)
		add_test_case(run_test_to_array_to_config run_test_to_array_to_config.cpp)
	endif()

	add_library(ad5933_config STATIC
		${AD5933_CONFIG_SRCS}
	)

	target_include_directories(ad5933_config PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/include
	)
endif()