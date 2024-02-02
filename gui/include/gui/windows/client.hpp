#pragma once

#include <memory>
#include <stop_token>
#include <vector>
#include <string>
#include <cstddef>
#include <atomic>
#include <memory>

#include "imgui.h"

#include "ad5933/data/data.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "ad5933/measurement/measurement.hpp"
#include "gui/run.hpp"
#include "gui/windows/captures.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "gui/windows/calibrate.hpp"
#include "gui/windows/plots/calibration.hpp"
#include "gui/windows/measure.hpp"
#include "gui/windows/plots/measurement.hpp"

namespace GUI {
    namespace Windows {
        struct Client {
            bool enable = true;
            std::string name;
            size_t index;
            Windows::Calibrate calibrate_window;
            Windows::Measure measure_window;
            Windows::Plots::Calibration calibration_plots_window;
            Windows::Plots::Measurement measurement_plots_window;
            Windows::Captures::Measurement configure_captures;
            std::vector<AD5933::Data> raw_calibration;
            std::vector<AD5933::Calibration<float>> calibration;
            std::vector<AD5933::Data> raw_measurement;
            std::vector<AD5933::Measurement<float>> measurement;
            bool calibrating = false;
            bool calibrated = false;
            bool sweeping = false;
            bool sweeped = false;
            bool periodically_sweeping = false;
            bool configured = false;
            std::shared_ptr<std::atomic<float>> progress_bar_fraction { std::make_shared<std::atomic<float>>(0.0f) };
            Windows::Captures::HexDebugReadWriteRegisters debug_captures;
            Client(const std::string name, const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm);
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