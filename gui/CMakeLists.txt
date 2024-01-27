cmake_minimum_required(VERSION 3.13)
set(CURRENT_PROJECT_NAME "gui")
project(${CURRENT_PROJECT_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(TRIELO_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/trielo_wrapper/trielo)
set(SIMPLEBLE_DIR ${CMAKE_CURRENT_LIST_DIR}/common/SimpleBLE/simpleble)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_TESTS OFF)
set(SDL_DISABLE_INSTALL ON)
set(SDL_DISABLE_INSTALL_DOCS ON)
set(SDL_INSTALL_TESTS OFF)
set(SDL3_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/SDL3)

set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/imgui)
set(IMPLOT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/implot)
set(IMGUI_CONSOLE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/imgui-console)
set(MAGIC_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/magic)
set(AD5933_MASKS_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/masks)
set(AD5933_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/config)
set(AD5933_MEASUREMENT_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/measurement)
set(AD5933_TEMPERATURE_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/temperature)
set(AD5933_DEBUG_DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/debug_data)
set(BLE_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR}/ble_client)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS date_time filesystem)

set(BOOST_SML_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/sml_wrapper/sml)
add_subdirectory(${BOOST_SML_DIR} ${CMAKE_BINARY_DIR}/sml)

set(IMGUI_SRCS
	${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
	${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
)

set(IMPLOT_SRCS
	${IMPLOT_DIR}/implot.cpp
	${IMPLOT_DIR}/implot_items.cpp
	${IMPLOT_DIR}/implot_demo.cpp
)

set(IMGUI_CONSOLE_SRCS
	${IMGUI_CONSOLE_DIR}/src/imgui_console.cpp
)

add_subdirectory(${TRIELO_DIR} ${CMAKE_BINARY_DIR}/trielo)
add_subdirectory(${SIMPLEBLE_DIR})
add_subdirectory(${MAGIC_DIR} ${CMAKE_BINARY_DIR}/magic)
add_subdirectory(${SDL3_DIR})
add_subdirectory(${AD5933_CONFIG_DIR} ${CMAKE_BINARY_DIR}/AD5933_config)
add_subdirectory(${AD5933_MEASUREMENT_DIR} ${CMAKE_BINARY_DIR}/AD5933_measurement)
add_subdirectory(${AD5933_TEMPERATURE_DIR} ${CMAKE_BINARY_DIR}/AD5933_temperature)
add_subdirectory(${AD5933_DEBUG_DATA_DIR} ${CMAKE_BINARY_DIR}/AD5933_debug_data)
add_subdirectory(${BLE_CLIENT_DIR})

add_executable(${CURRENT_PROJECT_NAME}
	main.cpp
	ble_client/lib/shm.cpp
	lib/ble_client/ble_client.cpp
	lib/gui/dark_mode_switcher.cpp
	lib/gui/imgui_sdl.cpp
	lib/gui/boilerplate.cpp
	lib/gui/spinner.cpp
	lib/gui/windows/connecting.cpp
	${IMGUI_SRCS}
	${IMPLOT_SRCS}
	${IMGUI_CONSOLE_SRCS}
)

target_include_directories(${CURRENT_PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/ble_client/include
	${IMGUI_DIR}
	${IMPLOT_DIR}
	${IMGUI_CONSOLE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${CURRENT_PROJECT_NAME} PRIVATE
	trielo
	simpleble::simpleble
	magic
	SDL3::SDL3-static
	AD5933_config
	AD5933_calibration
	AD5933_measurement
	AD5933_temperature
	AD5933_debug_data
    ${Boost_LIBRARIES}
	sml
)

if(WIN32)
	target_compile_definitions(${CURRENT_PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
endif()

