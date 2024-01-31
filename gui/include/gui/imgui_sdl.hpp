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

    struct MenuBarEnables {
        bool ble_client = true;
        bool configure = true;
        bool calibrate = true;
        bool measure = true;
        bool debug_registers = true;
        bool file_manager = true;
        bool measurement_plots = true;
        bool calibration_plots = true;
        bool console = true;
    };

    void run(
        bool &done,
        boost::process::child& ble_client,
        std::shared_ptr<BLE_Client::SHM::ParentSHM> shm
    );
}