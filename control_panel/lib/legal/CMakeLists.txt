cmake_minimum_required(VERSION 3.28)
project("legal" CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(Python3 COMPONENTS Interpreter)

execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/misc/license.py"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE PYTHON_SCRIPT_RESULT
    OUTPUT_VARIABLE PYTHON_SCRIPT_OUTPUT
    ERROR_VARIABLE PYTHON_SCRIPT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
)

# Check the result of running the Python script
if(PYTHON_SCRIPT_RESULT EQUAL 0)
    message(STATUS "Python script executed successfully")
    message(STATUS "Output: ${PYTHON_SCRIPT_OUTPUT}")
else()
    message(FATAL_ERROR "Error running Python script: ${PYTHON_SCRIPT_ERROR}")
endif()

add_library(${PROJECT_NAME} lib/legal.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)