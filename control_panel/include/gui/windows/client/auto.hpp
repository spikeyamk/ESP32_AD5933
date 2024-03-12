#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <queue>
#include <stop_token>
#include <optional>

#include "imgui.h"

#include "ble_client/shm/parent/parent.hpp"
#include "magic/events/results.hpp"
#include "gui/windows/client/lock.hpp"

namespace GUI {
    namespace Windows {
        class Auto {
        public:
            static constexpr std::u8string_view name_base { u8"Auto##" };
        private:
            bool first { true };
            size_t index;
            std::u8string name { name_base };
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
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
                CreatingTestFiles,

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
            enum class PopupShows {
                Off,
                Format,
                Remove,
            };
            PopupShows popup_shows { PopupShows::Off };
            struct Popup {
                bool shown { false };
                std::string name;
                std::string content;
                Popup(const std::string& name, const std::string& content) :
                    name { name },
                    content { content }
                {}
            };
            std::optional<Popup> popup { std::nullopt };
        private:
            struct ListTable {
                std::vector<Magic::Events::Results::File::List> paths;
                std::vector<Magic::Events::Results::File::Size> sizes;
            };
            ListTable list_table {};
            std::optional<size_t> selected { std::nullopt };
            struct Bytes {
                uint64_t used { 0 };
                uint64_t total { 0 };
            };
            Bytes bytes {};
            float progress_bar_fraction { 0.0f };
            uint16_t tick_ms_min { 0 };
            uint16_t tick_ms { 0 };
            uint16_t tick_ms_max { std::numeric_limits<uint16_t>::max() };
        public:
            struct Point {
                double time;
                Magic::Events::Results::Auto::Point auto_meas;
            };
            std::queue<Point> send_points;
            std::queue<Point> save_points;
            Auto() = default;
            Auto(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
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
            void draw_popups();
            void list();
            void remove();
            void download();
            void create_test_files();
            void format();
        private:
            static void remove_cb(Auto& self);
            static void list_cb(std::stop_token st, Auto& self);
            static void download_cb(std::stop_token st, Auto& self);
            static void create_test_files_cb(std::stop_token st, Auto& self);
            static void format_cb(std::stop_token st, Auto& self);
        };
    }
}