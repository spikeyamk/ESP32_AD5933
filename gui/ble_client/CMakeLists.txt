cmake_minimum_required(VERSION 3.13)
project(ble_client C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(WIN32)
	# Adding stupid macro definition so that boost doesn't complain about stupid Windows 7
    add_compile_definitions(_WIN32_WINNT=0x0A00)
endif()

find_package(Boost COMPONENTS date_time filesystem)
set(BOOST_SML_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/sml_wrapper/sml)
if(NOT TARGET trielo)
    set(TRIELO_DIR ${CMAKE_CURRENT_LIST_DIR}/../../common/trielo_wrapper/trielo)
    add_subdirectory(${TRIELO_DIR} ${CMAKE_BINARY_DIR}/ble_client/trielo)
endif()
if(NOT TARGET simpleble::simpleble)
    set(SIMPLE_BLE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/SimpleBLE/simpleble)
    add_subdirectory(${SIMPLE_BLE_DIR})
endif()

add_subdirectory(${BOOST_SML_DIR} ${CMAKE_BINARY_DIR}/ble_client/sml)

add_executable(ble_client main.cpp
	lib/init.cpp
	lib/ostream_overloads.cpp
	lib/state_machine.cpp
	lib/states.cpp
	lib/esp32_ad5933.cpp
)

target_link_libraries(ble_client PRIVATE
    ${Boost_LIBRARIES}
    sml
    simpleble::simpleble
    trielo
)

target_include_directories(ble_client PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
    ${Boost_INCLUDE_DIRS}
)
