#pragma once

#include <memory>

#include "imgui.h"

#include "gui/windows/client.hpp"
#include "ble_client/shm.hpp"

namespace GUI {
    namespace Windows {
        void file_manager(int i, ImGuiID side_id, bool &enable, Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
    }
}
