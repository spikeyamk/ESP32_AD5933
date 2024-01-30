#pragma once

#include <memory>
#include <vector>

#include "ble_client/standalone/shm.hpp"
#include "gui/windows/client.hpp"

namespace GUI {
    namespace Windows {
        void ble_client(
            bool &enable,
            ImGuiID left_id,
            int &selected,
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm,
            std::vector<Windows::Client>& client_windows
        );
    }
}
