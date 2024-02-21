#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <queue>
#include <stop_token>

#include "imgui.h"

#include "ble_client/shm/parent/parent.hpp"
#include "magic/events/results.hpp"

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
                Sending,
                Saving,
                Error,
            };
            Status status { Status::Off };
        public:
            struct Point {
                double time;
                Magic::Events::Results::Auto::Point auto_meas;
            };
            std::queue<Point> send_points;
            Auto() = default;
            Auto(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
            void draw(bool &enable, const ImGuiID side_id);
        private:
            void start_saving();
            void start_sending();
            void stop();
        private:
            std::stop_source stop_source;
            static void sending_cb(std::stop_token st, Auto& self);
        };
    }
}