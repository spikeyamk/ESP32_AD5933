cmake_minimum_required(VERSION 3.13)
project(ad5933_config C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(BOOST_MULTIPRECISION_DIR "lib/multiprecision")
set(BOOST_MP_STANDALONE ON)
add_subdirectory(${BOOST_MULTIPRECISION_DIR})

add_executable(main main.cpp
	lib/foo.cpp
)

target_include_directories(main PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
	${BOOST_MULTIPRECISION_DIR}/include
)

target_link_libraries(main PRIVATE 
	Boost::multiprecision
)
