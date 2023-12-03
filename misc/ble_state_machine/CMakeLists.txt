cmake_minimum_required(VERSION 3.13)
project(cpp-template C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(main main.cpp
	lib/magic_packets.cpp
	lib/packet_types.cpp
	lib/ble_states.cpp
	lib/ble_state_machine.cpp
	)

target_include_directories(main PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
	)