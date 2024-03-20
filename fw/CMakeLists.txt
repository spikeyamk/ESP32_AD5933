# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
    "main/lib"
    "main/lib/ad5933"
    "../common"
    "../common/ad5933"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_ad5933_fw)