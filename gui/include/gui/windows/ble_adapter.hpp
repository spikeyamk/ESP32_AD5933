#pragma once

#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <cstddef>
#include <optional>

#include "imgui.h"

#include "ble_client/shm/parent/parent.hpp"
#include "gui/windows/client/client.hpp"

namespace GUI {
    namespace Windows {
        class BLE_Adapter {
        private:
            size_t index;
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
            bool first { true };
            std::optional<size_t> selected { std::nullopt };
            std::vector<Windows::Client>& client_windows;
        public:
            static constexpr std::u8string_view name { u8"BLE Adapter" };
            BLE_Adapter(std::shared_ptr<BLE_Client::SHM::ParentSHM> shm, std::vector<Windows::Client>& client_windows);
            void draw(bool &enable, const ImGuiID left_id);
        private:
            void show_table();
        };
    }
}