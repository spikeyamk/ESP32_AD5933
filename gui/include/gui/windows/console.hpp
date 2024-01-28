#pragma once

#include <memory>
#include <ble_client/standalone/shm.hpp>

#include "imgui.h"

namespace GUI {
    namespace Windows {
        void console(
            bool &enable,
            ImGuiID bottom_id,
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm
        );
    }
}
