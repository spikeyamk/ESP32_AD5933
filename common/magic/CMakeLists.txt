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
		list(${MAGIC_SRCS} APPEND lib/misc/gettimeofday.cpp)
	endif()

	add_library(magic STATIC
		${MAGIC_SRCS}
	)

    target_include_directories(magic PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
	)

	option(MAGIC_TEST "Enable Magic Tests" OFF)

	if(MAGIC_TEST)
		function(add_test_case TEST_NAME TEST_SOURCE)
			add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
			target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
			add_test(NAME MagicPackets::Tests::${TEST_NAME}
					COMMAND ${TEST_NAME})
		endfunction()

		enable_testing()
		set(TEST_SRCS 
			${MAGIC_SRCS}
		)
		add_test_case(event_variant_map event_variant_map.cpp)
	endif()
endif()