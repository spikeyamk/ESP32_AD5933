cmake_minimum_required(VERSION 3.13)
set(CURRENT_PROJECT_NAME "ble_client")
project(${CURRENT_PROJECT_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS date_time filesystem)

if(NOT TARGET trielo)
    set(TRIELO_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/trielo_wrapper/trielo)
    add_subdirectory(${TRIELO_DIR} ${CMAKE_BINARY_DIR}/trielo)
endif()

if(NOT TARGET magic)
    set(MAGIC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/magic)
    add_subdirectory(${MAGIC_DIR} ${CMAKE_BINARY_DIR}/magic)
endif()

if(NOT TARGET simpleble::simpleble)
    set(SIMPLE_BLE_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/SimpleBLE/simpleble)
    add_subdirectory(${SIMPLE_BLE_DIR} ${CMAKE_BINARY_DIR}/simpleble)
endif()

if(NOT TARGET sml)
    set(BOOST_SML_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/sml_wrapper/sml)
    add_subdirectory(${BOOST_SML_DIR} ${CMAKE_BINARY_DIR}/sml)
endif()

if(NOT TARGET nlohmann_json)
	set(JSON_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib/extern/json)
    add_subdirectory(${JSON_DIR} ${CMAKE_BINARY_DIR}/json)
endif()

set(BLE_CLIENT_SRCS
	lib/init.cpp
	lib/shm.cpp
	lib/ostream_overloads.cpp
	lib/esp32_ad5933.cpp
	lib/cmd_listener.cpp
	lib/worker.cpp
	lib/test.cpp
	lib/state_machines/adapter/adapter.cpp
	lib/state_machines/adapter/states.cpp
	lib/state_machines/adapter/checker.cpp
	lib/state_machines/killer/killer.cpp
	lib/state_machines/connection/connection.cpp
	lib/state_machines/connector/connector.cpp
	${CMAKE_CURRENT_LIST_DIR}/../lib/json/shm.cpp
)

set(BLE_CLIENT_LIBS
    ${Boost_LIBRARIES}
    sml
    simpleble::simpleble
    trielo
	magic
	nlohmann_json
)

set(BLE_CLIENT_INCLUDE_DIRS
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/../include
    ${Boost_INCLUDE_DIRS}
)

add_executable(${CURRENT_PROJECT_NAME}
    main.cpp
	${BLE_CLIENT_SRCS}
)

target_link_libraries(${CURRENT_PROJECT_NAME} PRIVATE
	${BLE_CLIENT_LIBS}
)

target_include_directories(${CURRENT_PROJECT_NAME} PRIVATE
	${BLE_CLIENT_INCLUDE_DIRS}
)

if(WIN32)
	target_compile_definitions(${CURRENT_PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
endif()

option(BLE_CLIENT_TEST "Enable BLE_Client Tests" OFF)

if(BLE_CLIENT_TEST)
	function(add_test_case TEST_NAME TEST_SOURCE)
		add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
		target_link_libraries(${TEST_NAME} PRIVATE ${BLE_CLIENT_LIBS})
		target_include_directories(${TEST_NAME} PRIVATE ${BLE_CLIENT_INCLUDE_DIRS})
		add_test(NAME BLE_Client::Tests::${TEST_NAME}
				COMMAND ${TEST_NAME})
	endfunction()

	enable_testing()
	set(TEST_SRCS 
		${BLE_CLIENT_SRCS}
	)
	add_test_case(ble_client_basic_test basic_test.cpp)
endif()
