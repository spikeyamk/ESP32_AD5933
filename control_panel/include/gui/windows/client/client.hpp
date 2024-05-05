#pragma once

#include <memory>
#include <string>
#include <cstddef>
#include <memory>

#include "ble_client/shm/shm.hpp"
#include "gui/windows/popup_queue.hpp"
#include "gui/top.hpp"
#include "gui/windows/client/calibrate.hpp"
#include "gui/windows/client/plots/calibration.hpp"
#include "gui/windows/client/measure.hpp"
#include "gui/windows/client/plots/measurement.hpp"
#include "gui/windows/client/debug.hpp"
#include "gui/windows/client/auto.hpp"
#include "gui/windows/client/plots/auto.hpp"
#include "gui/windows/client/lock.hpp"

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
            std::shared_ptr<BLE_Client::SHM::SHM> shm { nullptr };
            Windows::PopupQueue* popup_queue;
            Windows::Calibrate calibrate_window;
            Windows::Plots::Calibration calibration_plots_window;
            Windows::Measure measure_window;
            Windows::Plots::Measurement measurement_plots_window;
            Windows::Debug debug_window;
            Windows::Auto auto_window;
            Windows::Plots::Auto auto_plots_window;
        public:
            Client(const std::string name, const size_t index, std::shared_ptr<BLE_Client::SHM::SHM> parent_shm, Windows::PopupQueue* popup_queue);
            void draw(const ImGuiID center_id, Top::MenuBarEnables &enables);
            const std::string& get_address() const;
        };
    }
}