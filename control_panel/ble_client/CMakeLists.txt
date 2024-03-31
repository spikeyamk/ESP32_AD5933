cmake_minimum_required(VERSION 3.28)
project("ble_client" C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

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
	lib/state_machines/adapter/adapter.cpp
	lib/state_machines/adapter/states.cpp
	lib/state_machines/adapter/checker.cpp
	lib/state_machines/killer/killer.cpp
	lib/state_machines/connection/connection.cpp
	lib/state_machines/connector/connector.cpp
)

set(BLE_CLIENT_INCLUDE_DIRS
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/../include
)

add_library(${PROJECT_NAME}
	${BLE_CLIENT_SRCS}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	#target_compile_options(${PROJECT_NAME} /W4)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
    sml
    simpleble::simpleble
    trielo
	magic
	nlohmann_json
)

target_include_directories(${PROJECT_NAME} PUBLIC
	${BLE_CLIENT_INCLUDE_DIRS}
)

if(WIN32)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
endif()

option(BLE_CLIENT_TEST "Enable BLE_Client Tests" OFF)

if(BLE_CLIENT_TEST)
	if(NOT TARGET AD5933_config)
		set(AD5933_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/ad5933/config)
		add_subdirectory(${AD5933_CONFIG_DIR} ${CMAKE_BINARY_DIR}/ad5933_config)
	endif()

	if(NOT TARGET AD5933_data)
		set(AD5933_DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/ad5933/data)
		add_subdirectory(${AD5933_DATA_DIR} ${CMAKE_BINARY_DIR}/ad5933_data)
	endif()

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
	
	enable_testing()
	set(TEST_SRCS 
		"tests/tests.cpp"
		"lib/shm/common/names.cpp"
		lib/shm/parent/specialized.cpp
		lib/shm/parent/parent.cpp
	)
	set(TEST_INCLUDE_DIRS
		${CMAKE_CURRENT_LIST_DIR}/tests/include
		${BLE_CLIENT_INCLUDE_DIRS}
	)
	set(TEST_LINK_LIBS
		magic
		AD5933_config
		AD5933_data
		${PROJECT_NAME}
	)
	#add_test_case(ble_client_thread_connect_sync_scan thread/connect/sync_scan.cpp)
	#add_test_case(ble_client_thread_connect_async_scan thread/connect/async_scan.cpp)

	#add_test_case(ble_client_thread_debug_dump thread/debug/dump.cpp)
	#add_test_case(ble_client_thread_debug_program_and_dump thread/debug/program_and_dump.cpp)

	#add_test_case(ble_client_thread_auto_send thread/auto/send.cpp)
	add_test_case(ble_client_thread_freq_sweep_periodic thread/freq_sweep/periodic.cpp)

	#add_test_case(ble_client_thread_file_free thread/file/free.cpp)
	#add_test_case(ble_client_thread_file_list_count thread/file/list_count.cpp)
	#add_test_case(ble_client_thread_file_list thread/file/list.cpp)
	#add_test_case(ble_client_thread_file_size thread/file/size.cpp)
	#add_test_case(ble_client_thread_file_remove thread/file/remove.cpp)
	#add_test_case(ble_client_thread_file_download thread/file/download.cpp)
endif()
