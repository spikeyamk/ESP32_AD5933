#pragma once

#include <memory>
#include <string>

#include "imgui.h"

#include "gui/windows/captures.hpp"
#include "ble_client/shm/parent/parent.hpp"

namespace GUI {
    namespace Windows {
        class Debug {
        private:
            bool first { true };
            size_t index;
            std::string name { "Debug##" };
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
        public:
            enum class Status {
                NotDumped,
                Dumped,
            };
        private:
            Status status { Status::NotDumped };
            Windows::Captures::HexDebugReadWriteRegisters debug_captures {};
        public:
            Status get_status() const;
            Debug() = default;
            Debug(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
            void draw(bool &enable, const ImGuiID side_id);
        private:
            void draw_input_elements();
            bool dump();
            bool program_and_dump();
            bool command_and_dump(const AD5933::Masks::Or::Ctrl::HB::Command command);
        };
    }
}
