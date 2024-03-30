#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <cstddef>
#include <vector>
#include <stop_token>

#include "imgui.h"

#include "ad5933/masks/masks.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ad5933/config/config.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "json/conversion.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "gui/windows/client/lock.hpp"
#include "misc/channel.hpp"

namespace GUI {
    namespace Windows {
        class Calibrate {
        public:
            static constexpr std::u8string_view name_base { u8"Calibrate##" };
        private:
            struct Min {
                static constexpr uint32_t freq_start { 1'000 };
                static constexpr uint32_t freq_inc { 1 };
                static constexpr uint32_t num_of_inc { 1 };
                static constexpr uint32_t settling_cycles { 1 };
            };
            struct Max {
                static constexpr uint32_t freq { 100'000 };
                static constexpr uint32_t freq_start { 99'999 };
                uint32_t freq_inc { 99'000 };
                static constexpr uint32_t nine_bit { 511 };
                uint16_t num_of_inc { nine_bit };
                static constexpr uint16_t settling_cycles { nine_bit };
            };
            Max max {};
            struct Fields {
                uint32_t freq_start { 30'000 };
                uint32_t freq_inc { 10 };
                uint16_t num_of_inc { 2 };
                float impedance { 1'000.0f };
                int voltage_range_combo { 0 };
                int pga_gain_combo { 0 };
                int sysclk_src_combo { 0 };
                int settling_number { 15 };
                int settling_multiplier_combo { 0 };
                uint32_t sysclk_freq { 16'667'000 };
            };
            Fields fields {};
            size_t index;
            std::u8string name { name_base };
            bool first { true };
        public:
            AD5933::Config config {
                AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode,
                AD5933::Masks::Or::Ctrl::HB::VoltageRange::Two_Vppk,
                AD5933::Masks::Or::Ctrl::HB::PGA_Gain::OneTime,
                AD5933::Masks::Or::Ctrl::LB::SysClkSrc::Internal,

                AD5933::uint_startfreq_t { 30'000 },
                AD5933::uint_incfreq_t { 10 },
                AD5933::uint9_t { 2 },

                AD5933::uint9_t { 15 },
                AD5933::Masks::Or::SettlingTimeCyclesHB::Multiplier::OneTime
            };
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
            std::shared_ptr<Channel<ns::CalibrationFile>> calibration_queue_to_load_into_measurement { std::make_shared<Channel<ns::CalibrationFile>>() };
            Calibrate(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
            ~Calibrate();
            void draw(bool& enable, const ImGuiID side_id, Lock& lock);
            Status get_status() const;
            bool plotted { false };
        private:
            const std::optional<Lock> draw_inner();
            void draw_input_fields();
            void send();
        private:
            static void calibrate_cb(std::stop_token st, Calibrate& self);
            void calibrate();
            void save() const;
        };
    }
}
