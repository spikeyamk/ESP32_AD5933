if(ESP_PLATFORM)
	idf_component_register(SRCS
						   "lib/packets.cpp"
                       	   INCLUDE_DIRS "include"
	)
else()
	cmake_minimum_required(VERSION 3.13)
	project(magic_packets C CXX ASM)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_BUILD_TYPE Debug)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	add_library(magic STATIC
		lib/packets.cpp
	)

	target_include_directories(magic PRIVATE
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
			lib/packets.cpp
			tests/tests.cpp
		)
		add_test_case(run_packet_duplication run_packet_duplication.cpp)
		add_test_case(run_get_magic_packet_pointer run_get_magic_packet_pointer.cpp)
		add_test_case(run_check_for_0xFF run_check_for_0xFF.cpp)
	endif()
endif()

