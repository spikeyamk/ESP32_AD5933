cmake_minimum_required(VERSION 3.13)
project(cpp-template C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(TRIELO_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/trielo)
set(SIMPLEBLE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/SimpleBLE/simpleble)
set(SDL2_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/SDL2)
set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/imgui)
set(IMPLOT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/implot)

add_subdirectory(${TRIELO_DIR})
add_subdirectory(${SIMPLEBLE_DIR})
add_subdirectory(${SDL2_DIR})

add_executable(main main.cpp
	lib/ad5933_config.cpp
	lib/ble.cpp
	lib/dark_mode_switcher.cpp
	lib/imgui_sdl.cpp
	lib/magic_packets.cpp
	${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
	${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMPLOT_DIR}/implot.cpp
	${IMPLOT_DIR}/implot_items.cpp
)

target_include_directories(main PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
	${TRIELO_DIR}/include
	${SIMPLEBLE_DIR}/include
	${SDL2_DIR}/include
	${IMGUI_DIR}
	${IMPLOT_DIR}
)

target_link_libraries(main PRIVATE
	trielo
	simpleble::simpleble
	SDL2::SDL2main
	SDL2::SDL2-static
	windowsapp
)