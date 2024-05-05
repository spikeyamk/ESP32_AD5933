if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/calibration.cpp"
                       	   INCLUDE_DIRS "include"
						   REQUIRES ad5933_data
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(AD5933_calibration C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(AD5933_CALIBRATION_SRCS
		lib/calibration.cpp
	)

	set(AD5933_DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/../ad5933_data)
	if(NOT TARGET AD5933_data)
		add_subdirectory(${AD5933_DATA_DIR} ${CMAKE_BINARY_DIR}/data)
	endif()
	set(AD5933_CALIBRATION_LINK_LIBS AD5933_data)

	option(AD5933_CALIBRATION_TEST "Enable AD5933::Calibration Tests" OFF)
	if(AD5933_CALIBRATION_TEST)
		set(TEST_SRCS 
			lib/tests.cpp
		)

		list(APPEND AD5933_CALIBRATION_SRCS ${TEST_SRCS})

		function(add_test_case TEST_NAME TEST_SOURCE)
			add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
			target_include_directories(${TEST_NAME} PRIVATE
				${CMAKE_CURRENT_LIST_DIR}/include
			)
			target_link_libraries(${TEST_NAME} PRIVATE
				${AD5933_CALIBRATION_LINK_LIBS}
			)
			add_test(NAME AD5933::Calibration::Tests::${TEST_NAME}
					COMMAND ${TEST_NAME})
		endfunction()

		enable_testing()
		add_test_case(run_test_constructor run_test_constructor.cpp)
	endif()

	add_library(AD5933_calibration STATIC
		${AD5933_CALIBRATION_SRCS}
	)

	target_include_directories(AD5933_calibration PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)
	target_link_libraries(AD5933_calibration PUBLIC
		${AD5933_CALIBRATION_LINK_LIBS}
	)
endif()
