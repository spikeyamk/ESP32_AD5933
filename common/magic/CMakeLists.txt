if(ESP_PLATFORM)
	idf_component_register(SRCS
		tests/tests.cpp
		INCLUDE_DIRS "include")
else()
	cmake_minimum_required(VERSION 3.13)
	project(magic_packets C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	set(MAGIC_SRCS
		tests/tests.cpp
	)

	if(WIN32)
		list(APPEND MAGIC_SRCS lib/misc/gettimeofday.cpp)
	endif()

	if(NOT TARGET serde)
		set(SERDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../serde_wrapper/serde)
		add_subdirectory(${SERDE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/serde)
	endif()

	add_library(magic STATIC
		${MAGIC_SRCS}
	)

    target_include_directories(magic PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)

    target_link_libraries(magic PUBLIC
		serde
	)

	option(MAGIC_TEST "Enable Magic Tests" OFF)

	if(MAGIC_TEST)
		function(add_test_case TEST_NAME TEST_SOURCE)
			add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
			target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
			target_link_libraries(${TEST_NAME} PRIVATE magic)
			add_test(NAME MagicPackets::Tests::${TEST_NAME}
					COMMAND ${TEST_NAME})
		endfunction()

		enable_testing()
		set(TEST_SRCS 
			${MAGIC_SRCS}
		)
		add_test_case(commands_with_serde commands_with_serde.cpp)
		add_test_case(results_with_serde results_with_serde.cpp)
	endif()
endif()