#pragma once

#include <esp_err.h>
#include <filesystem>
#include <string_view>

namespace SD_Card {
    constexpr std::string_view mount_point { "/sdcard" };
    constexpr std::string_view mount_point_prefix { "/sdcard/" };
    int init();
    void deinit();
    esp_err_t format();
    void create_test_files();
    void print_test_files();
}