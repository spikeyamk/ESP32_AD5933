#pragma once

#include <memory>

#include <imgui.h>

#include "ble_client/shm/parent/parent.hpp"

namespace GUI {
    struct DockspaceIDs {
        ImGuiID left;
        ImGuiID center;
    };

    void run(
        bool &done,
        std::shared_ptr<BLE_Client::SHM::Parent> shm
    );
}