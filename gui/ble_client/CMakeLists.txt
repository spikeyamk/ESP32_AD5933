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
    add_subdirectory(${TRIELO_DIR} ${CMAKE_BINARY_DIR}/ble_client/trielo)
endif()

if(NOT TARGET simpleble::simpleble)
    set(SIMPLE_BLE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/SimpleBLE/simpleble)
    add_subdirectory(${SIMPLE_BLE_DIR})
endif()

if(NOT TARGET sml)
    set(BOOST_SML_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/sml_wrapper/sml)
    add_subdirectory(${BOOST_SML_DIR} ${CMAKE_BINARY_DIR}/ble_client/sml)
endif()

add_executable(${CURRENT_PROJECT_NAME}
    main.cpp
	lib/init.cpp
	lib/ostream_overloads.cpp
	lib/state_machine.cpp
	lib/states.cpp
	lib/esp32_ad5933.cpp
	lib/state_machines/adapter/adapter.cpp
	lib/state_machines/killer/killer.cpp
	lib/state_machines/connection/connection.cpp
)

target_link_libraries(${CURRENT_PROJECT_NAME} PRIVATE
    ${Boost_LIBRARIES}
    sml
    simpleble::simpleble
    trielo
)

target_include_directories(${CURRENT_PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

if(WIN32)
	target_compile_definitions(${CURRENT_PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
endif()
