if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/data.cpp"
                       	   INCLUDE_DIRS "include"
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(AD5933_data C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(AD5933_DATA_SRCS
		lib/data.cpp
	)

	option(AD5933_DATA_TEST "Enable AD5933::Data Tests" OFF)
	if(AD5933_DATA_TEST)
		set(TEST_SRCS 
			lib/tests.cpp
		)

		list(APPEND AD5933_DATA_SRCS ${TEST_SRCS})

		function(add_test_case TEST_NAME TEST_SOURCE)
			add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
			target_include_directories(${TEST_NAME} PRIVATE
				${CMAKE_CURRENT_LIST_DIR}/include
			)
			add_test(NAME AD5933::Data::Tests::${TEST_NAME}
					COMMAND ${TEST_NAME})
		endfunction()

		enable_testing()
		add_test_case(run_test_array_constructor run_test_array_constructor.cpp)
		add_test_case(run_test_raw_magnitude run_test_raw_magnitude.cpp)
		add_test_case(run_test_raw_phase run_test_raw_phase.cpp)
	endif()

	add_library(AD5933_data STATIC
		${AD5933_DATA_SRCS}
	)

	target_include_directories(AD5933_data PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)
endif()
