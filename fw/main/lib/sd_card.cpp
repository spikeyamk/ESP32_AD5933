/* SD card and FAT filesystem example.
   This example uses SPI peripheral to communicate with SD card.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <thread>

#include <trielo/trielo.hpp>

#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "esp_littlefs.h"
#include <trielo/trielo.hpp>

#include "sd_card.hpp"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.

namespace SD_Card {
    namespace Pins {
        static constexpr gpio_num_t miso { GPIO_NUM_15 };
        static constexpr gpio_num_t mosi { GPIO_NUM_23 };
        static constexpr gpio_num_t clk  { GPIO_NUM_22 };
        static constexpr gpio_num_t cs   { GPIO_NUM_21 };
    }

    sdmmc_card_t card {};

    esp_vfs_littlefs_conf_t littlefs_conf {
        .base_path = mount_point.data(),
        .partition_label = nullptr,
        .partition = nullptr,
        .sdcard = &card,
        .format_if_mount_failed = 1,
        .read_only = 0,
        .dont_mount = 0,
        .grow_on_mount = 1
    };

    // This is just = SDSPI_HOST_DEFAULT() but max_freq_khz modified
    static constexpr sdmmc_host_t host {
        .flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG,
        .slot = SDSPI_DEFAULT_HOST,
        .max_freq_khz = 20'000,
        .io_voltage = 3.3f,
        .init = &sdspi_host_init,
        .set_bus_width = NULL,
        .get_bus_width = NULL,
        .set_bus_ddr_mode = NULL,
        .set_card_clk = &sdspi_host_set_card_clk,
        .set_cclk_always_on = NULL,
        .do_transaction = &sdspi_host_do_transaction,
        .deinit_p = &sdspi_host_remove_device,
        .io_int_enable = &sdspi_host_io_int_enable,
        .io_int_wait = &sdspi_host_io_int_wait,
        .command_timeout_ms = 0,
        .get_real_freq = &sdspi_host_get_real_freq,
        .input_delay_phase = SDMMC_DELAY_PHASE_0,
        .set_input_delay = NULL
    };

    static constexpr spi_bus_config_t bus_cfg {
        .mosi_io_num = Pins::mosi,
        .miso_io_num = Pins::miso,
        .sclk_io_num = Pins::clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    // This corresponds to SDSPI_DEVICE_CONFIG_DEFAULT() just .host_id and .gpio_cs is modified
    static constexpr sdspi_device_config_t slot_config {
        .host_id   = spi_host_device_t(host.slot),
        .gpio_cs   = Pins::cs,
        .gpio_cd   = SDSPI_SLOT_NO_CD,
        .gpio_wp   = SDSPI_SLOT_NO_WP,
        .gpio_int  = GPIO_NUM_NC,
        .gpio_wp_polarity = SDSPI_IO_ACTIVE_LOW,
    };

    sdspi_dev_handle_t handle;

    int init() {
        if(Trielo::trielo<spi_bus_initialize>(Trielo::OkErrCode(ESP_OK), slot_config.host_id, &bus_cfg, SDSPI_DEFAULT_DMA)) {
            return -1;
        }

        if(Trielo::trielo<sdspi_host_init>(Trielo::OkErrCode(ESP_OK)) != ESP_OK) {
            return -2;
        }

        if(Trielo::trielo<sdspi_host_init_device>(Trielo::OkErrCode(ESP_OK), &slot_config, &handle) != ESP_OK) {
            return -3;
        }
        
        if(Trielo::trielo<sdmmc_card_init>(Trielo::OkErrCode(ESP_OK), &host, &card) != ESP_OK) {
            return -4;
        }

        Trielo::trielo<sdmmc_card_print_info>(stdout, &card);

        if(Trielo::trielo<esp_vfs_littlefs_register>(Trielo::OkErrCode(ESP_OK), &littlefs_conf) != ESP_OK) {
            return -5;
        }

        return 0;
    }

    esp_err_t format() {
        return esp_littlefs_format_sdmmc(&card);
    }

    int create_test_files() {
        const std::filesystem::path test1 { std::filesystem::path(mount_point).append("test1") };
        const std::filesystem::path test2 { std::filesystem::path(mount_point).append("test2") };
        const std::filesystem::path test3 { std::filesystem::path(mount_point).append("test3") };
        std::ofstream file1(test1);
        int ret { -1 };
        if(file1.is_open()) {
            const char text[19] = "Lorem Ipsum is sim";
            file1 << text;
            file1.close();
            ret = 2;
        }
        std::ofstream file2(test2);
        if(file2.is_open()) {
            const char text[] = "Lorem Ipsum is simply dummy text of the printing and typesetting industry.";
            file2 << text;
            file2.close();
            ret = 1;
        }
        std::ofstream file3(test3);
        if(file3.is_open()) {
            const char text[] = "abba";
            file3 << text;
            file3.close();
            ret = 0;
        }
        return ret;
    }

    void print_test_files() {
        const std::filesystem::path test1 { std::filesystem::path(mount_point).append("test1") };
        const std::filesystem::path test2 { std::filesystem::path(mount_point).append("test2") };
        const std::filesystem::path test3 { std::filesystem::path(mount_point).append("test3") };
        std::ifstream file1(test1);
        if(file1.is_open()) {
            std::cout << test1 << ": \n";
            std::cout << file1.rdbuf() << std::endl;
        }
        std::ifstream file2(test2);
        if(file2.is_open()) {
            std::cout << test2 << ": \n";
            std::cout << file2.rdbuf() << std::endl;
        }
        std::ifstream file3(test3);
        if(file3.is_open()) {
            std::cout << test3 << ": \n";
            std::cout << file3.rdbuf() << std::endl;
        }
    }

    esp_err_t deinit() {
        esp_err_t ret { Trielo::trielo<esp_vfs_littlefs_unregister_sdmmc>(Trielo::OkErrCode(ESP_OK), &card) };
        if(ret != ESP_OK) {
            return ret;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        ret = Trielo::trielo<sdspi_host_remove_device>(Trielo::OkErrCode(ESP_OK), handle);
        if(ret != ESP_OK) {
            return ret;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        ret = Trielo::trielo<sdspi_host_deinit>(Trielo::OkErrCode(ESP_OK));
        if(ret != ESP_OK) {
            return ret;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return Trielo::trielo<spi_bus_free>(Trielo::OkErrCode(ESP_OK), slot_config.host_id);
    }

    int create_test_file(const size_t size_bytes, const std::string_view& name) {
        const std::filesystem::path test_megabyte { std::filesystem::path(mount_point).append(name) };

        std::ofstream file(test_megabyte);
        if(file.is_open() == false) {
            return -1;
        }
        size_t i = 1;
        try {
            char c { 'a' };
            for(; i <= size_bytes; i++) {
                file << c;
                if(c == 'z') {
                    c = 'a';
                } else {
                    c++;
                }
                if((i % 512) == 0) {
                    file.flush();
                }
            }
        } catch(const std::exception& e) {
            return -2;
        }

        if(i < size_bytes) {
            return static_cast<int>(i);
        }
        return 0;
    }

    int print_test_file(const std::string_view& name) {
        const std::filesystem::path test_megabyte { std::filesystem::path(mount_point).append(name) };
        static constexpr size_t size_megabyte { 1 * 1024 * 1024 };
        std::ifstream file(test_megabyte);
        if(file.is_open() == false) {
            return -1;
        }
        char c;
        size_t i = 1;
        while(file.read(&c, sizeof(c))) {
            std::printf("%c", c);
            i++;
        }
        std::printf("\n");
        if(i < size_megabyte) {
            return static_cast<int>(i);
        }

        return 0;
    }
}
