cmake_minimum_required(VERSION 3.28)
set(CURRENT_PROJECT_NAME "ble_client")
project(${CURRENT_PROJECT_NAME} C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT Boost_FOUND)
	set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDebug)
	set(Boost_USE_STATIC_LIBS    ON)
	set(Boost_USE_MULTITHREADED  ON)
	set(Boost_USE_STATIC_RUNTIME ON)
	find_package(Boost COMPONENTS date_time filesystem)
endif()

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
	lib/ostream_overloads.cpp
	lib/esp32_ad5933.cpp
	lib/cmd_listener.cpp
	lib/child_main.cpp
	"lib/shm/common/names.cpp"
	lib/shm/child/specialized.cpp
	lib/shm/child/child.cpp
	lib/state_machines/adapter/adapter.cpp
	lib/state_machines/adapter/states.cpp
	lib/state_machines/adapter/checker.cpp
	lib/state_machines/killer/killer.cpp
	lib/state_machines/connection/connection.cpp
	lib/state_machines/connector/connector.cpp
)

set(BLE_CLIENT_LINK_LIBS
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

add_library(${CURRENT_PROJECT_NAME}
	${BLE_CLIENT_SRCS}
)

target_link_libraries(${CURRENT_PROJECT_NAME} PUBLIC
	${BLE_CLIENT_LINK_LIBS}
)

target_include_directories(${CURRENT_PROJECT_NAME} PUBLIC
	${BLE_CLIENT_INCLUDE_DIRS}
)

if(WIN32)
	target_compile_definitions(${CURRENT_PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
endif()

option(BLE_CLIENT_TEST "Enable BLE_Client Tests" OFF)

if(BLE_CLIENT_TEST)
	function(add_test_case TEST_NAME TEST_SOURCE)
		add_executable(${TEST_NAME} ${TEST_SRCS} tests/${TEST_SOURCE})
		target_include_directories(${TEST_NAME} PRIVATE ${TEST_INCLUDE_DIRS})
		target_link_libraries(${TEST_NAME} PRIVATE ${TEST_LINK_LIBS})
		if(WIN32)
			target_compile_definitions(${TEST_NAME} PRIVATE _WIN32_WINNT=0x0A00)
		endif()
		add_test(NAME BLE_Client::Tests::${TEST_NAME}
				COMMAND ${TEST_NAME})

	endfunction()
	
	list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
	enable_testing()
	set(TEST_SRCS 
		"tests/tests.cpp"
		"lib/shm/common/names.cpp"
		lib/shm/common/clean.cpp
		lib/shm/parent/specialized.cpp
		lib/shm/parent/parent.cpp
	)
	set(TEST_INCLUDE_DIRS
		${CMAKE_CURRENT_LIST_DIR}/tests/include
		${BLE_CLIENT_INCLUDE_DIRS}
	)
	set(TEST_LINK_LIBS
		${BLE_CLIENT_LINK_LIBS}
		${CURRENT_PROJECT_NAME}
		AD5933_config
	)
	add_test_case(ble_client_connect_sync_scan connect/sync_scan.cpp)
	add_test_case(ble_client_connect_async_scan connect/async_scan.cpp)
	add_test_case(ble_client_connect_interprocess_sync_scan connect/interprocess_sync_scan.cpp)
	add_test_case(ble_client_connect_interprocess_async_scan connect/interprocess_async_scan.cpp)
	add_test_case(ble_client_debug_dump debug/dump.cpp)
	add_test_case(ble_client_debug_program_and_dump debug/program_and_dump.cpp)
	add_test_case(ble_client_debug_interprocess_dump debug/interprocess_dump.cpp)
	add_test_case(ble_client_file_free file/free.cpp)
	add_test_case(ble_client_file_list_count file/list_count.cpp)
	add_test_case(ble_client_file_list file/list.cpp)
	add_test_case(ble_client_file_size file/size.cpp)
	add_test_case(ble_client_file_remove file/remove.cpp)
	add_test_case(ble_client_file_download file/download.cpp)
endif()
