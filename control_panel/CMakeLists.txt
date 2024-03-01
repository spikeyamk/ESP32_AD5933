cmake_minimum_required(VERSION 3.13)
set(CURRENT_PROJECT_NAME "control_panel")
project(${CURRENT_PROJECT_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# Makes UTF-8 glyphs work
	add_compile_options("/utf-8")
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
set(JSON_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/json)
set(NATIVE_FILE_DIALOG_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/nativefiledialog-extended)
set(SEMVER_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/semver)
set(UTFCONV_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/utfconv)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS date_time filesystem)

set(BOOST_SML_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/sml_wrapper/sml)
add_subdirectory(${BOOST_SML_DIR} ${CMAKE_BINARY_DIR}/sml)

# This is disabled because we embed the font in the binary
#configure_file(
#	${CMAKE_CURRENT_LIST_DIR}/misc/UbuntuSans-Regular.ttf
#	${CMAKE_BINARY_DIR}/UbuntuSans-Regular.ttf
#	COPYONLY
#)

set(IMGUI_SRCS
	${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
	${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

set(IMPLOT_SRCS
	${IMPLOT_DIR}/implot.cpp
	${IMPLOT_DIR}/implot_items.cpp
	${IMPLOT_DIR}/implot_demo.cpp
)

add_subdirectory(${TRIELO_DIR} ${CMAKE_BINARY_DIR}/trielo)
add_subdirectory(${SIMPLEBLE_DIR})
add_subdirectory(${MAGIC_DIR} ${CMAKE_BINARY_DIR}/magic)
add_subdirectory(${SDL3_DIR})
add_subdirectory(${AD5933_CONFIG_DIR} ${CMAKE_BINARY_DIR}/AD5933_config)
add_subdirectory(${AD5933_MEASUREMENT_DIR} ${CMAKE_BINARY_DIR}/AD5933_measurement)
add_subdirectory(${AD5933_TEMPERATURE_DIR} ${CMAKE_BINARY_DIR}/AD5933_temperature)
add_subdirectory(${AD5933_DEBUG_DATA_DIR} ${CMAKE_BINARY_DIR}/AD5933_debug_data)
add_subdirectory(${JSON_DIR})
add_subdirectory(${NATIVE_FILE_DIALOG_DIR})
# This ugly hack with external semver library lets us use the mbits libraries without needing to install
add_subdirectory(${SEMVER_DIR})
# This just creates an alias that the CMakeLists.txt inside the ${UTFCONV_DIR} is looking for
add_library(mbits::semver ALIAS semver)
add_subdirectory(${UTFCONV_DIR})

add_subdirectory(${BLE_CLIENT_DIR})

set(GUI_SRCS
	main.cpp
	ble_client/lib/shm/common/clean.cpp
	"ble_client/lib/shm/common/names.cpp"
	ble_client/lib/shm/parent/specialized.cpp
	ble_client/lib/shm/parent/parent.cpp
	lib/imgui_custom/spinner.cpp
	lib/imgui_custom/input_items.cpp
	lib/imgui_custom/char_filters.cpp
	lib/imgui_custom/markdown.cpp

	lib/gui/run.cpp
	lib/gui/top.cpp
	lib/gui/boilerplate.cpp
	lib/gui/ubuntu_sans_regular.cpp

	# singleton windows
	lib/gui/windows/console.cpp
	lib/gui/windows/ble_adapter.cpp

	# dynamic windows
	lib/gui/windows/client/client.cpp
	lib/gui/windows/client/calibrate.cpp
	lib/gui/windows/client/measure.cpp
	lib/gui/windows/client/file_manager.cpp
	lib/gui/windows/client/debug.cpp
	lib/gui/windows/client/auto.cpp

	# dynamic windows plots
	lib/gui/windows/client/plots/measurement.cpp
	lib/gui/windows/client/plots/calibration.cpp
	lib/gui/windows/client/plots/auto.cpp

	lib/json/conversion.cpp
	lib/json/settings.cpp

	lib/legal/boost.cpp
	lib/legal/dear_imgui.cpp
	lib/legal/implot.cpp
	lib/legal/json.cpp
	lib/legal/nativefiledialog_extended.cpp
	lib/legal/sdl3.cpp
	lib/legal/semver.cpp
	lib/legal/simpleble.cpp
	lib/legal/sml.cpp
	lib/legal/ubuntu_sans_fonts.cpp
	lib/legal/utfconv.cpp
	lib/legal/fmt.cpp
	${IMGUI_SRCS}
	${IMPLOT_SRCS}
)

set(GUI_INCLUDE_DIRS
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/ble_client/include
	${IMGUI_DIR}
	${IMPLOT_DIR}
    ${Boost_INCLUDE_DIRS}
)

set(GUI_LINK_LIBRARIES
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
	nfd
	nlohmann_json 
	ble_client
	utfconv
)

add_executable(${CURRENT_PROJECT_NAME}
	${GUI_SRCS}
)

target_include_directories(${CURRENT_PROJECT_NAME} PRIVATE
	${GUI_INCLUDE_DIRS}
)

target_link_libraries(${CURRENT_PROJECT_NAME} PRIVATE
	${GUI_LINK_LIBRARIES}
)

if(WIN32)
	target_compile_definitions(${CURRENT_PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
endif()

if(SHM_CLEANER)
	set(SHM_CLEANER shm_cleaner)

	add_executable(${SHM_CLEANER}
		misc/shm_cleaner.cpp
		"ble_client/lib/shm/common/names.cpp"
		ble_client/lib/shm/common/clean.cpp
	)

	target_link_libraries(${SHM_CLEANER} PRIVATE
		trielo
		nlohmann_json
		magic
		${Boost_LIBRARIES}
	)

	target_include_directories(${SHM_CLEANER} PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/ble_client/include
		${Boost_INCLUDE_DIRS}
	)

	if(WIN32)
		target_compile_definitions(${SHM_CLEANER} PRIVATE _WIN32_WINNT=0x0A00)
	endif()
endif()