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

#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>

#include "sd_card.hpp"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.

namespace SD_Card {
    namespace Pins {
        static constexpr gpio_num_t miso { GPIO_NUM_2 };
        static constexpr gpio_num_t mosi { GPIO_NUM_10 };
        static constexpr gpio_num_t clk  { GPIO_NUM_11 };
        static constexpr gpio_num_t cs   { GPIO_NUM_3 };
    }

    static constexpr esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false
    };

    sdmmc_card_t* card;

    // This is just = SDSPI_HOST_DEFAULT() but max_freq_khz modified
    static constexpr sdmmc_host_t host {
        .flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG,
        .slot = SDSPI_DEFAULT_HOST,
        .max_freq_khz = 400, // Here the max_freq_khz is modifed to 400 kHz
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

    int init() {
        if(spi_bus_initialize(slot_config.host_id, &bus_cfg, SDSPI_DEFAULT_DMA) != ESP_OK) {
            return -1;
        }

        if(esp_vfs_fat_sdspi_mount(mount_point.data(), &host, &slot_config, &mount_config, &card) != ESP_OK) {
            return -2;
        }
        sdmmc_card_print_info(stdout, card);
        return 0;
    }

    esp_err_t format() {
        return esp_vfs_fat_sdcard_format(mount_point.data(), card);
    }

    void create_test_files() {
        const std::filesystem::path test1 { std::filesystem::path(mount_point).append("test1") };
        const std::filesystem::path test2 { std::filesystem::path(mount_point).append("test2") };
        const std::filesystem::path test3 { std::filesystem::path(mount_point).append("test3") };
        std::ofstream file1(test1);
        if(file1.is_open()) {
            const char text[19] = "Lorem Ipsum is sim";
            file1 << text;
            file1.close();
        }
        std::ofstream file2(test2);
        if(file2.is_open()) {
            const char text[] = "Lorem Ipsum is simply dummy text of the printing and typesetting industry.";
            file2 << text;
            file2.close();
        }
        std::ofstream file3(test3);
        if(file3.is_open()) {
            const char text[] = "abba";
            file3 << text;
            file3.close();
        }
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

    void deinit() {
        esp_vfs_fat_sdcard_unmount(mount_point.data(), card);
        spi_bus_free(slot_config.host_id);
    }
}
