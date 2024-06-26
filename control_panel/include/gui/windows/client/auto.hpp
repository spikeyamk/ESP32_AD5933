#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <stop_token>

#include "imgui.h"

#include "ble_client/shm/shm.hpp"
#include "magic/results/results.hpp"
#include "gui/windows/client/lock.hpp"
#include "gui/windows/popup_queue.hpp"
#include "misc/channel.hpp"

namespace GUI {
    namespace Windows {
        class Auto {
        public:
            static constexpr std::u8string_view name_base { u8"Auto##" };
        private:
            bool first { true };
            size_t index;
            std::u8string name { name_base };
            std::shared_ptr<BLE_Client::SHM::SHM> shm { nullptr };
            PopupQueue* popup_queue { nullptr };
            std::string address;
        public:
            enum class Status {
                Off,

                // for Control
                Sending,
                Saving,

                // for Records
                Listing,
                Downloading,
                Removing,
                Formatting,

                Error,
            };
            Status status { Status::Off };

            enum class Error {
                Off,

                // for Control
                Send,
                Save,

                // for Records
                List,
                Download,
                Remove,
                Format,
                CreateTestFiles,
            };
            Error error { Error::Off };
        private:
            struct ListTable {
                std::vector<Magic::Results::File::List> paths;
                std::vector<Magic::Results::File::Size> sizes;
            };
            ListTable list_table {};
            std::optional<size_t> selected { std::nullopt };
            struct Bytes {
                uint64_t total { 0 };
                uint64_t used { 0 };
                uint64_t free { 0 };
                inline void update(const Magic::Results::File::Free& free_result) {
                    total = free_result.total_bytes;
                    used = free_result.used_bytes;
                    free = total - used;
                }
            };
            Bytes bytes {};
            float progress_bar_fraction { 0.0f };
            static constexpr uint16_t tick_ms_min { 0 };
            uint16_t tick_ms { tick_ms_min };
            uint16_t tick_ms_max { std::numeric_limits<uint16_t>::max() };
        public:
            struct Point {
                double time;
                Magic::Results::Auto::Point auto_meas;
            };
            std::shared_ptr<Channel<Point>> send_points { std::make_shared<Channel<Point>>() };
            std::shared_ptr<Channel<Point>> save_points { std::make_shared<Channel<Point>>() };
            Auto() = default;
            Auto(const size_t index, std::shared_ptr<BLE_Client::SHM::SHM> shm, PopupQueue* popup_queue, const std::string& address);
            ~Auto();
            void draw(bool &enable, const ImGuiID side_id, Lock& lock);
        private:
            const std::optional<Lock> draw_inner();
            void start_saving();
            void start_sending();
            void stop();
        private:
            std::stop_source stop_source;
            static void start_sending_cb(std::stop_token st, Auto& self);
        private:
            void draw_list_table();
            void draw_list_table_rows();
            void draw_bytes_table();
            void list();
            void remove();
            void download();
            void format();
        private:
            static void remove_cb(Auto& self);
            static void list_cb(std::stop_token st, Auto& self);
            static void download_cb(std::stop_token st, Auto& self);
            static void format_cb(std::stop_token st, Auto& self);
        };
    }
}