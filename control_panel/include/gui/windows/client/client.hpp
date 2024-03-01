#pragma once

#include <memory>
#include <stop_token>
#include <vector>
#include <string>
#include <cstddef>
#include <atomic>
#include <memory>
#include <string_view>
#include <optional>
#include <thread>

#include <SDL3/SDL.h>
#include "imgui.h"

#include "ad5933/data/data.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "ad5933/measurement/measurement.hpp"
#include "gui/run.hpp"
#include "ble_client/shm/parent/parent.hpp"

#include "gui/windows/client/calibrate.hpp"
#include "gui/windows/client/plots/calibration.hpp"
#include "gui/windows/client/measure.hpp"
#include "gui/windows/client/plots/measurement.hpp"
#include "gui/windows/client/file_manager.hpp"
#include "gui/windows/client/debug.hpp"
#include "gui/windows/client/auto.hpp"
#include "gui/windows/client/plots/auto.hpp"
#include "gui/windows/client/lock.hpp"
#include "gui/top.hpp"

namespace GUI {
    namespace Windows {
        class Client {
        private:
            std::string address;
            bool first { true };
            std::string name;
        public:
            bool enable { true };
            size_t index;
            Lock lock { Lock::Released };
        private:
            std::string dockspace_name;
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
            Windows::Calibrate calibrate_window;
            Windows::Plots::Calibration calibration_plots_window;
            Windows::Measure measure_window;
            Windows::Plots::Measurement measurement_plots_window;
            Windows::FileManager file_manager_window;
            Windows::Debug debug_window;
            Windows::Auto auto_window;
            Windows::Plots::Auto auto_plots_window;
        public:
            Client(const std::string name, const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> parent_shm);
            void draw(const ImGuiID center_id, Top::MenuBarEnables &enables);
            const std::string& get_address() const;
        };
    }
}