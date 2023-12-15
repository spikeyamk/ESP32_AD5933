if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/calibration.cpp"
                       	   INCLUDE_DIRS "include"
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(AD5933_temperature C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(AD5933_TEMPERATURE_SRCS
		lib/temperature.cpp
	)

	option(AD5933_TEMPERATURE_TEST "Enable AD5933::Calibration Tests" OFF)
	if(AD5933_TEMPERATURE_TEST)
		set(TEST_SRCS 
			lib/tests.cpp
		)

		list(APPEND AD5933_TEMPERATURE_SRCS ${TEST_SRCS})

		function(add_test_case TEST_NAME TEST_SOURCE)
			add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
			target_include_directories(${TEST_NAME} PRIVATE
				${CMAKE_CURRENT_LIST_DIR}/include
			)
			add_test(NAME AD5933::Temperature::Tests::${TEST_NAME}
					COMMAND ${TEST_NAME})
		endfunction()

		enable_testing()
		add_test_case(run_test_constructor run_test_constructor.cpp)
	endif()

	add_library(AD5933_temperature STATIC
		${AD5933_TEMPERATURE_SRCS}
	)

	target_include_directories(AD5933_temperature PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)
endif()
