#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <cstddef>
#include <vector>
#include <stop_token>
#include <optional>
#include <mutex>

#include "imgui.h"

#include "ad5933/masks/masks.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ad5933/config/config.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "json/conversion.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "gui/windows/client/lock.hpp"

namespace GUI {
    namespace Windows {
        class Calibrate {
        public:
            static constexpr std::u8string_view name_base { u8"Calibrate##" };
        private:
            static constexpr uint32_t min_freq = 1'000;
            static constexpr uint32_t max_freq = 100'000;
            static constexpr uint32_t res_freq = 1;
            static constexpr uint32_t max_9bit = 511;
            struct ValidTextInputs {
                bool freq_start = true;
                bool freq_inc = true;
                bool num_of_inc = true;
                bool impedance = true;
                bool settling_number = true;
                inline bool all_true() const {
                    return freq_start && freq_inc && num_of_inc && impedance && settling_number;
                }
            };
            ValidTextInputs valid_fields {};
            size_t index;
            std::u8string name { name_base };
            bool first { true };
        private:
            struct GUI_ItemInputs {
                char freq_start[7] { "30000" };
                char freq_inc[7] { "10" };
                char num_of_inc[4] { "2" };
                int voltage_range_combo { 0 };
                int pga_gain_combo { 0 };
                int settling_multiplier_combo { 0 };
                char settling_number[4] { "15" };
                int sysclk_src_combo { 0 };
            };

            struct NumericValues {
                uint32_t freq_start { 30'000 };
                uint32_t freq_inc { 10 };
                uint16_t num_of_inc { 2 };
                float impedance { 1'000.0f };
                float impedance_before { impedance };
                uint16_t settling_number { 15 };
                uint32_t sysclk_freq { 16'667'000 };
            };

            struct Inputs {
                GUI_ItemInputs gui {};
                NumericValues numeric_values {};
            };

            Inputs inputs {};
        public:
            AD5933::Config config {};
            enum class Status {
                NotCalibrated,
                Calibrating,
                Failed,
                Calibrated,
                Plotted,
            };
        private:
            std::stop_source stop_source;
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm;
        public:
            std::vector<AD5933::Data> raw_calibration {};
            std::vector<AD5933::Calibration<float>> calibration {};
        private:
            Status status { Status::NotCalibrated };
            ns::CalibrationFile calibration_file;
            float progress_bar_fraction { 0.0f };
        public:
            Calibrate(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
            ~Calibrate();
            void draw(bool& enable, const ImGuiID side_id, Lock& lock);
            Status get_status() const;
            bool plotted { false };
        private:
            const std::optional<Lock> draw_inner();
            void draw_input_fields();
            void update_freq_start_valid();
            void update_freq_inc_valid();
            void update_num_of_inc_valid();
            void update_impedance_valid();
            void update_settling_number_valid();
        private:
            static void calibrate_cb(std::stop_token st, Calibrate& self);
            void calibrate();
            void save() const;
        };
    }
}
