if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/measurement.cpp"
                       	   INCLUDE_DIRS "include"
						   REQUIRES data calibration
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(AD5933_measurement C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(AD5933_MEASUREMENT_SRCS lib/measurement.cpp)

	set(AD5933_CALIBRATION_DIR ${CMAKE_CURRENT_LIST_DIR}/../calibration)
	add_subdirectory(${AD5933_CALIBRATION_DIR} ${CMAKE_BINARY_DIR}/calibration)
	set(AD5933_MEASUREMENT_LINK_LIBS 
		AD5933_data
		AD5933_calibration
	)

	option(AD5933_MEASUREMENT_TEST "Enable AD5933::Measurement Tests" OFF)
	if(AD5933_MEASUREMENT_TEST)
		set(TEST_SRCS 
			lib/tests.cpp
		)

		list(APPEND AD5933_MEASUREMENT_SRCS ${TEST_SRCS})

		function(add_test_case TEST_NAME TEST_SOURCE)
			add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
			target_include_directories(${TEST_NAME} PRIVATE
				${CMAKE_CURRENT_LIST_DIR}/include
			)
			target_link_libraries(${TEST_NAME} PRIVATE
				${AD5933_MEASUREMENT_LINK_LIBS}
			)
			add_test(NAME AD5933::Measurement::Tests::${TEST_NAME}
					COMMAND ${TEST_NAME})
		endfunction()

		enable_testing()
		add_test_case(run_test_constructor run_test_constructor.cpp)
	endif()

	add_library(AD5933_measurement STATIC
		${AD5933_MEASUREMENT_SRCS}
	)

	target_include_directories(AD5933_measurement PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)
	target_link_libraries(AD5933_measurement PRIVATE
		${AD5933_MEASUREMENT_LINK_LIBS}
	)
endif()
