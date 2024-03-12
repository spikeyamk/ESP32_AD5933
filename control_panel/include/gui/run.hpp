#pragma once

#include <optional>
#include <memory>

#include <boost/process.hpp>
#include "imgui.h"

#include "ble_client/shm/parent/parent.hpp"

namespace GUI {
    struct DockspaceIDs {
        ImGuiID left;
        ImGuiID center;
    };

    void run(
        bool &done,
        boost::process::child& ble_client,
        std::shared_ptr<BLE_Client::SHM::ParentSHM> shm
    );
}