cmake_minimum_required(VERSION 3.22)
project(serde CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(lib/extern/pfr)
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    "include"
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    boost_pfr
)

option(SERDE_TEST "Enable Serde Tests" OFF)

if(SERDE_TEST)
	enable_testing()

	add_library("${PROJECT_NAME}_tests" STATIC
		tests/tests.cpp
	)

	target_link_libraries("${PROJECT_NAME}_tests" PRIVATE serde)
	target_include_directories("${PROJECT_NAME}_tests" INTERFACE include)

	if(WIN32)
		target_compile_definitions("${PROJECT_NAME}_tests" PRIVATE _WIN32_WINNT=0x0A00)
	endif()

	function(serde_add_test SERDE_TEST_NAME)
		add_executable("${PROJECT_NAME}_tests_${SERDE_TEST_NAME}"
			"tests/${SERDE_TEST_NAME}.cpp"
		)

		if(WIN32)
			target_compile_definitions("${PROJECT_NAME}_tests_${SERDE_TEST_NAME}" PRIVATE _WIN32_WINNT=0x0A00)
		endif()

		target_link_libraries("${PROJECT_NAME}_tests_${SERDE_TEST_NAME}" PRIVATE
			"${PROJECT_NAME}_tests"
		)	

		add_test(
			NAME "Serde::Tests::${SERDE_TEST_NAME}"
			COMMAND "${PROJECT_NAME}_tests_${SERDE_TEST_NAME}"
		)
	endfunction()

	serde_add_test(object_super)
	serde_add_test(object_super_iterable)
	serde_add_test(object_super_iterable_empty)
endif()
