#pragma once

#include <atomic>
#include <vector>
#include <optional>
#include <memory>
#include <stop_token>

#include "imgui.h"
#include "ad5933/data/data.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "ad5933/measurement/measurement.hpp"
#include "gui/windows/captures.hpp"
#include "gui/windows/client.hpp"
#include "gui/imgui_sdl.hpp"

#include "ble_client/standalone/shm.hpp"

namespace GUI {
    namespace Windows {
        void ble_client(bool &enable, ImGuiID left_id, int &selected, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm, std::vector<Windows::Client>& client_windows);
    }
}