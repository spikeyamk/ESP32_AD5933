#pragma once

#include <string>
#include <memory>

#include "imgui.h"

#include "ble_client/shm/parent/parent.hpp"

namespace GUI {
    namespace Windows {
        class Auto {
        private:
            bool first { true };
            size_t index;
            std::string name { "Auto##" };
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
        public:
            enum class Status {
                Off,
                Sending,
                Saving,
            };
            Status status { Status::Off };
        public:
            Auto() = default;
            Auto(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
            void draw(bool &enable, const ImGuiID side_id);
        private:
            void start_saving();
            void start_sending();
            void stop();
            void stop_saving();
            void stop_sending();
        };
    }
}