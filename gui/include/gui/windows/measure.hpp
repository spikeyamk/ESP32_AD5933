#pragma once

#include <memory>

#include "imgui.h"

#include "gui/windows/client.hpp"
#include "ble_client/standalone/shm.hpp"

namespace GUI {
    namespace Windows {
        void measure(int i, ImGuiID side_id, bool &enable, Client &client, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
    }
}
