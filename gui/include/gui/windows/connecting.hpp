#pragma once

#include <atomic>
#include <vector>
#include <optional>
#include <memory>
#include <stop_token>

#include "imgui.h"
#include "imgui_console/imgui_console.h"
#include "ad5933/data/data.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "ad5933/measurement/measurement.hpp"
#include "gui/windows/captures.hpp"

#include "ble_client/ble_client.hpp"

namespace GUI {
    namespace Windows {
        struct DockspaceIDs {
            ImGuiID left;
            ImGuiID center;
        };
        struct MenuBarEnables {
            bool ble_client = true;
            bool configure = true;
            bool debug_registers = true;
            bool measurement_plots = true;
            bool calibration_plots = true;
        };
        struct Client {
            bool enable = true;
            std::shared_ptr<ESP32_AD5933> esp32_ad5933;
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
            float progress_bar_fraction { 0.0f };
            Windows::Captures::HexDebugReadWriteRegisters debug_captures;
            inline Client(std::shared_ptr<ESP32_AD5933> esp32_ad5933) :
                esp32_ad5933 {esp32_ad5933}
            {}
            bool debug_started = false;
            std::stop_source ss;
        };
        void ble_client(std::optional<SimpleBLE::Adapter> &adapter, bool &enable, ImGuiID left_id, std::vector<SimpleBLE::Peripheral> &peripherals, int &selected, std::vector<Windows::Client> &clients);
        void console(ImGuiConsole &console);
        void client1(int i, ImGuiID center_id, Client &client, MenuBarEnables &enables);
        ImGuiID top_with_dock_space(MenuBarEnables &menu_bar_enables);
        DockspaceIDs split_left_center(ImGuiID dockspace_id);
    }
}