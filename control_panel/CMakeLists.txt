cmake_minimum_required(VERSION 3.28)
project("esp32_ad5933_control_panel" C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# Makes UTF-8 work
	# Could not get it to work with target definitions has to be global
	add_compile_options("/utf-8")
	set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDebug)
endif()

set(TRIELO_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/trielo_wrapper/trielo)
set(SDL3_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/SDL)
set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/imgui)
set(IMPLOT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/implot)
set(MAGIC_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/magic)
set(AD5933_MASKS_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/ad5933_masks)
set(AD5933_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/ad5933_config)
set(AD5933_MEASUREMENT_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/ad5933_measurement)
set(AD5933_TEMPERATURE_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/ad5933_temperature)
set(AD5933_DEBUG_DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/ad5933/ad5933_debug_data)
set(BLE_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/ble_client)
set(LEGAL_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/legal)
set(JSON_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/json)
set(NATIVE_FILE_DIALOG_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/nativefiledialog-extended)
set(SEMVER_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/semver)
set(UTFCONV_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/utfconv)
set(UTFCONV_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/extern/utfconv)

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
	${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

set(IMPLOT_SRCS
	${IMPLOT_DIR}/implot.cpp
	${IMPLOT_DIR}/implot_items.cpp
	${IMPLOT_DIR}/implot_demo.cpp
)

add_subdirectory(${TRIELO_DIR} ${CMAKE_BINARY_DIR}/trielo)
add_subdirectory(${MAGIC_DIR} ${CMAKE_BINARY_DIR}/magic)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_TESTS OFF)
set(SDL_DISABLE_INSTALL ON)
set(SDL_DISABLE_INSTALL_DOCS ON)
set(SDL_INSTALL_TESTS OFF)
add_subdirectory(${SDL3_DIR})

add_subdirectory(${AD5933_CONFIG_DIR} ${CMAKE_BINARY_DIR}/AD5933_config)
add_subdirectory(${AD5933_MEASUREMENT_DIR} ${CMAKE_BINARY_DIR}/AD5933_measurement)
add_subdirectory(${AD5933_TEMPERATURE_DIR} ${CMAKE_BINARY_DIR}/AD5933_temperature)
add_subdirectory(${AD5933_DEBUG_DATA_DIR} ${CMAKE_BINARY_DIR}/AD5933_debug_data)
add_subdirectory(${JSON_DIR})
add_subdirectory(${NATIVE_FILE_DIALOG_DIR})
target_compile_definitions(nfd PRIVATE NFD_NATIVE)
add_subdirectory(${SEMVER_DIR})

# This just creates an alias that the CMakeLists.txt inside the ${UTFCONV_DIR} is looking for
# Getting rid of this dependency is of high priority
add_library(mbits::semver ALIAS semver)
add_subdirectory(${UTFCONV_DIR})
add_subdirectory(${BLE_CLIENT_DIR})
add_subdirectory(${LEGAL_DIR})

add_executable(${PROJECT_NAME}
	main.cpp

	lib/imgui_custom/spinner.cpp
	lib/imgui_custom/input_items.cpp
	lib/imgui_custom/char_filters.cpp

	lib/implot_custom/setup_axis_label.cpp

	lib/gui/run.cpp
	lib/gui/top.cpp
	lib/gui/boilerplate.cpp
	lib/gui/ubuntu_sans_regular.cpp

	# singleton windows
	lib/gui/windows/ble_adapter.cpp
	lib/gui/windows/implot_dense_test.cpp
	lib/gui/windows/popup_queue.cpp

	# dynamic windows
	lib/gui/windows/client/client.cpp
	lib/gui/windows/client/calibrate.cpp
	lib/gui/windows/client/measure.cpp
	lib/gui/windows/client/debug.cpp
	lib/gui/windows/client/auto.cpp

	# dynamic windows plots
	lib/gui/windows/client/plots/measurement.cpp
	lib/gui/windows/client/plots/calibration.cpp
	lib/gui/windows/client/plots/auto.cpp

	lib/json/conversion.cpp
	lib/json/settings.cpp

	${IMGUI_SRCS}
	${IMPLOT_SRCS}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# This macro _WIN32_WINNT is for some reason needed for using boost libraries because they can't detect the Windows version themselves?
	target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE NFD_NATIVE)
target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION="0.1.12")
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(${PROJECT_NAME} PRIVATE GIT_COMMIT_HASH="${GIT_COMMIT_HASH}")

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/ble_client/include
	${IMGUI_DIR}
	${IMPLOT_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	trielo
	magic
    SDL3::SDL3-static
	AD5933_config
	AD5933_calibration
	AD5933_measurement
	AD5933_temperature
	AD5933_debug_data
	sml
	nfd
	nlohmann_json 
	ble_client
	legal
	utfconv
	serde 
)