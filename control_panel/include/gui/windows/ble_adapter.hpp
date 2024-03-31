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
            std::shared_ptr<BLE_Client::SHM::Parent> shm { nullptr };
            bool first { true };
            std::optional<size_t> selected { std::nullopt };
            std::vector<Windows::Client>& client_windows;
            bool show_ble_off_error_pop_up { false };
            bool show_connection_attempt_timeout_error_pop_up { false };
            std::vector<std::stop_source> stop_sources;
        public:
            static constexpr std::u8string_view name { u8"BLE Adapter" };
            BLE_Adapter(std::shared_ptr<BLE_Client::SHM::Parent> shm, std::vector<Windows::Client>& client_windows);
            ~BLE_Adapter();
            void draw(bool &enable, const ImGuiID left_id);
        private:
            void show_table();
            void show_disabled_connect_button() const;
            void show_disabled_start_button() const;
            void validate_start_attempt();
        };
    }
}