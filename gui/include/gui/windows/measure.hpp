#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <stop_token>
#include <vector>
#include <queue>
#include <span>
#include <array>
#include <chrono>

#include "ad5933/masks/maps.hpp"
#include "imgui.h"

#include "ble_client/shm/parent/parent.hpp"
#include "json/conversion.hpp"
#include "ad5933/config/config.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "ad5933/measurement/measurement.hpp"

namespace GUI {
    namespace Windows {
        class Measure {
        private:
            bool first { true };
            size_t index;
            std::string name { "Measure##" };
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };

            static constexpr uint32_t res_freq = 1;
            static constexpr uint32_t max_9bit = 511;

            struct Inputs {
                struct GUI_ItemInputs {
                    int settling_multiplier_combo { 0 };
                };

                struct Numeric {
                    uint32_t freq_start;
                    uint32_t freq_end;
                    int settling_number { 15 };
                };

                GUI_ItemInputs gui {};
                Numeric numeric {};
            };
            Inputs inputs {};

            struct VaildTextInputs {
                bool freq_start { true };
                bool freq_end { true };
                bool freq_inc { true };
                bool settling_number { true };
                inline bool all_true() const {
                    return freq_start && freq_end && freq_inc && settling_number;
                }
            };
            VaildTextInputs valid_fields {};
            float progress_bar_fraction { 0.0f };
            bool periodically_sweeping { false };
        public:
            enum class Status {
                NotLoaded,
                Loaded,
                Measuring,
                Failed,
            };
            Status status { Status::NotLoaded };
            bool single_plotted { false };
            struct Configs {
                AD5933::Config calibration;
                AD5933::Config measurement;
            };
            Configs configs {};
            struct CalibrationVectors {
                std::vector<AD5933::Calibration<float>> calibration;
                std::vector<uint32_t> freq_uint32_t;
                std::vector<float> freq_float;
            };
            CalibrationVectors calibration_vectors {};
            struct SingleVectors {
                std::vector<AD5933::Data> raw_measurement;
                std::vector<AD5933::Measurement<float>> measurement;
                std::vector<float> freq_float;
            };
            SingleVectors single_vectors;
            struct PeriodicPoint {
                double time_point;
                std::vector<AD5933::Data> raw_measurement;
                std::vector<AD5933::Measurement<float>> measurement;
            };
            struct PeriodicVectors {
                std::vector<float> freq_float;
                std::queue<PeriodicPoint> periodic_points;
            };
            PeriodicVectors periodic_vectors;
            std::stop_source stop_source;
        public:
            Measure() = default;
            Measure(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
            void draw(bool &enable, const ImGuiID side_id);
        private:
            static void single_cb(std::stop_token st, Measure& self);
            void single();
            static void periodic_cb(std::stop_token st, Measure& self);
            void periodic();
            void stop();
            bool load();
            void draw_input_elements(); 
        };
    }
}
