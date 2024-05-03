#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <cstddef>
#include <optional>

#include "imgui.h"

#include "ble_client/shm/shm.hpp"
#include "gui/windows/client/client.hpp"
#include "gui/windows/popup_queue.hpp"

namespace GUI {
    namespace Windows {
        class BLE_Adapter {
        private:
            size_t index;
            std::shared_ptr<BLE_Client::SHM::SHM> shm { nullptr };
            bool first { true };
            std::optional<size_t> selected { std::nullopt };
            std::vector<Windows::Client>& client_windows;
            std::vector<std::stop_source> stop_sources;
            bool connecting { false };
            PopupQueue& popup_queue;
        public:
            static constexpr std::u8string_view name { u8"BLE Adapter" };
            BLE_Adapter(std::shared_ptr<BLE_Client::SHM::SHM> shm, std::vector<Windows::Client>& client_windows, PopupQueue& popup_queue);
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