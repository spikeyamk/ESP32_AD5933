#pragma once

#include <memory>
#include <stop_token>
#include <vector>
#include <string>
#include <cstddef>

#include "imgui.h"

#include "ad5933/data/data.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "ad5933/measurement/measurement.hpp"
#include "gui/imgui_sdl.hpp"
#include "gui/windows/captures.hpp"
#include "ble_client/standalone/shm.hpp"

namespace GUI {
    namespace Windows {
        struct Client {
            bool enable = true;
            Windows::Captures::Measurement configure_captures;
            std::vector<AD5933::Data> raw_calibration;
            std::vector<AD5933::Calibration<float>> calibration;
            std::vector<AD5933::Data> raw_measurement;
            std::vector<AD5933::Measurement<float>> measurement;
            std::string name;
            size_t index;
            bool calibrating = false;
            bool calibrated = false;
            bool sweeping = false;
            bool sweeped = false;
            bool periodically_sweeping = false;
            bool configured = false;
            float progress_bar_fraction { 0.0f };
            Windows::Captures::HexDebugReadWriteRegisters debug_captures;
            inline Client(const std::string name, const size_t index) :
                name{ name },
                index{ index }
            {}
            bool debug_started = false;
            std::stop_source ss;
        };

        void client1(
            const int i,
            ImGuiID center_id,
            Client &client,
            MenuBarEnables &enables,
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm
        );
    }
}