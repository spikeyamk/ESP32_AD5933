cmake_minimum_required(VERSION 3.13)
project(rcd2json C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT TARGET magic)
	set(MAGIC_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/magic)
	add_subdirectory(${MAGIC_DIR} ${CMAKE_BINARY_DIR}/magic)
endif()

if(NOT TARGET nlohmann)
	set(NLOHMANN_JSON_DIR ${CMAKE_CURRENT_LIST_DIR}/../control_panel/lib/extern/json)
	add_subdirectory(${NLOHMANN_JSON_DIR} ${CMAKE_BINARY_DIR}/json)
endif()

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
	magic
	nlohmann_json 
)

#target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
#	${CMAKE_CURRENT_LIST_DIR}/include
#)
