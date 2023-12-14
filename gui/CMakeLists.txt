cmake_minimum_required(VERSION 3.13)
project(cpp-template C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(TRIELO_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/trielo_wrapper/trielo)
set(SIMPLEBLE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/SimpleBLE/simpleble)
set(SDL2_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/SDL2)
set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/imgui)
set(IMPLOT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/implot)
set(MAGIC_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/magic)

set(IMGUI_SRCS
	${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
	${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
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

add_subdirectory(${TRIELO_DIR} ${CMAKE_BINARY_DIR}/trielo)
add_subdirectory(${SIMPLEBLE_DIR})
add_subdirectory(${MAGIC_DIR} ${CMAKE_BINARY_DIR}/magic)
add_subdirectory(${SDL2_DIR})

add_executable(main main.cpp
	lib/ad5933_config.cpp
	lib/ble.cpp
	lib/dark_mode_switcher.cpp
	lib/imgui_sdl.cpp
	${IMGUI_SRCS}
	${IMPLOT_SRCS}
)

target_include_directories(main PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
	${TRIELO_DIR}/include
	${SIMPLEBLE_DIR}/include
	${MAGIC_DIR}/include
	${SDL2_DIR}/include
	${IMGUI_DIR}
	${IMPLOT_DIR}
)

set(GUI_LINK_LIBS
	trielo
	simpleble::simpleble
	magic
	SDL2::SDL2main
	SDL2::SDL2-static
	#windowsapp
)

target_link_libraries(main PRIVATE
	${GUI_LINK_LIBS}
)
