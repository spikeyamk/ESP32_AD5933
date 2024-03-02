#pragma once

#include <string_view>

#include <esp_err.h>
#include <sdmmc_cmd.h>

namespace SD_Card {
    extern sdmmc_card_t card;
    constexpr std::string_view mount_point { "/sdcard" };
    constexpr std::string_view mount_point_prefix { "/sdcard/" };
    int init();
    esp_err_t deinit();
    esp_err_t format();
    int create_test_files();
    void print_test_files();
    int create_megabyte_test_file();
    int print_megabyte_test_file();
}