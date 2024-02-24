#pragma once

#include <cstdint>
#include <array>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <optional>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <memory>
#include <string>
#include <string_view>

#include "ad5933/masks/masks.hpp"
#include "ad5933/debug_data/debug_data.hpp"
#include "ad5933/masks/maps.hpp"
#include "ad5933/config/config.hpp"
#include "ad5933/uint_types.hpp"

#include "imgui.h"

#include "ble_client/shm/parent/parent.hpp"
#include "gui/windows/client/lock.hpp"

namespace GUI {
    namespace Windows {
        namespace Captures {
            // This is terrible, but it'll have to do for now. This class should be inside GUI::Windows::Debug, but due to C++ build system being crap it has to be here and templated. DON'T MOVE, otherwise it'll cause a segfault.
            template<size_t N>
            class Debug {
            public:
                std::array<char, N> ctrl_HB { 0x00 };
                std::array<char, N> ctrl_LB { 0x00 };
                std::array<char, N> freq_start_HB { 0x00 };
                std::array<char, N> freq_start_MB { 0x00 };
                std::array<char, N> freq_start_LB { 0x00 };
                std::array<char, N> freq_inc_HB { 0x00 };
                std::array<char, N> freq_inc_MB { 0x00 };
                std::array<char, N> freq_inc_LB { 0x00 };
                std::array<char, N> num_of_inc_HB { 0x00 };
                std::array<char, N> num_of_inc_LB { 0x00 };
                std::array<char, N> settling_time_cycles_HB { 0x00 };
                std::array<char, N> settling_time_cycles_LB { 0x00 };
                std::array<char, N> status_capture { 0x00 };
                std::array<char, N> temp_data_HB { 0x00 };
                std::array<char, N> temp_data_LB { 0x00 };
                std::array<char, N> real_data_HB { 0x00 };
                std::array<char, N> real_data_LB { 0x00 };
                std::array<char, N> imag_data_HB { 0x00 };
                std::array<char, N> imag_data_LB { 0x00 };
                std::array<std::array<char, N>*, 19> all { 
                    &ctrl_HB,
                    &ctrl_LB,
                    &freq_start_HB,
                    &freq_start_MB,
                    &freq_start_LB,
                    &freq_inc_HB,
                    &freq_inc_MB,
                    &freq_inc_LB,
                    &num_of_inc_HB,
                    &num_of_inc_LB,
                    &settling_time_cycles_HB,
                    &settling_time_cycles_LB,
                    &status_capture,
                    &temp_data_HB,
                    &temp_data_LB,
                    &real_data_HB,
                    &real_data_LB,
                    &imag_data_HB,
                    &imag_data_LB
                };

                AD5933::Config config {
                    AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode,
                    AD5933::Masks::Or::Ctrl::HB::VoltageRange::Two_Vppk,
                    AD5933::Masks::Or::Ctrl::HB::PGA_Gain::OneTime,
                    AD5933::Masks::Or::Ctrl::LB::SysClkSrc::Internal,
                    AD5933::uint_startfreq_t { 30'000 },
                    AD5933::uint_incfreq_t { 10 },
                    AD5933::uint9_t { 2 },
                    AD5933::uint9_t { 15 },
                    AD5933::Masks::Or::SettlingTimeCyclesHB::Multiplier::OneTime
                };
                AD5933::DebugData<float> data {};

                Debug() = default;

                Debug(const Debug &other) {
                    assert(all[0] == &ctrl_HB);
                    for(size_t i = 0; i < all.size(); i++) {
                        *(all[i]) = *(other.all[i]);
                    }
                    config = other.config;
                    data = other.data;
                }

                void update_config() {
                    auto stoul_lambda = [](const std::array<char, 5>& hex_array) -> std::optional<uint8_t> {
                        try {
                            // Extract the numeric value from the string
                            uint8_t result = static_cast<uint8_t>(std::stoul(std::string { hex_array.begin(), hex_array.end() }, nullptr, 16)); // Convert hexadecimal string to unsigned long
                            return result;
                        } catch (...) {
                            return std::nullopt; // Return empty optional on conversion failure
                        }
                    };
                    const std::array<uint8_t, 12> tmp_raw_array {
                        stoul_lambda(ctrl_HB).value_or(static_cast<uint8_t>(config.ctrl.HB.to_ulong())),
                        stoul_lambda(ctrl_LB).value_or(static_cast<uint8_t>(config.ctrl.LB.to_ulong())),
                        stoul_lambda(freq_start_HB).value_or(static_cast<uint8_t>(config.start_freq.HB.to_ulong())),
                        stoul_lambda(freq_start_MB).value_or(static_cast<uint8_t>(config.start_freq.MB.to_ulong())),
                        stoul_lambda(freq_start_LB).value_or(static_cast<uint8_t>(config.start_freq.MB.to_ulong())),
                        stoul_lambda(freq_inc_HB).value_or(static_cast<uint8_t>(config.inc_freq.HB.to_ulong())),
                        stoul_lambda(freq_inc_MB).value_or(static_cast<uint8_t>(config.inc_freq.MB.to_ulong())),
                        stoul_lambda(freq_inc_LB).value_or(static_cast<uint8_t>(config.inc_freq.LB.to_ulong())),
                        stoul_lambda(num_of_inc_HB).value_or(static_cast<uint8_t>(config.num_of_inc.HB.to_ulong())),
                        stoul_lambda(num_of_inc_LB).value_or(static_cast<uint8_t>(config.num_of_inc.LB.to_ulong())),
                        stoul_lambda(settling_time_cycles_HB).value_or(static_cast<uint8_t>(config.settling_time_cycles.HB.to_ulong())),
                        stoul_lambda(settling_time_cycles_LB).value_or(static_cast<uint8_t>(config.settling_time_cycles.LB.to_ulong())),
                    };

                    config = AD5933::Config { tmp_raw_array };
                }

                void update_captures(const std::string &in_data) {
                    std::array<std::string, 19> oss_array;
                    std::for_each(
                        oss_array.begin(),
                        oss_array.end(),
                        [index = 0, &in_data](std::string &e) mutable {
                            std::ostringstream temp_oss;
                            temp_oss << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(in_data[index]));
                            e = temp_oss.str();
                            index++;
                        }
                    );

                    std::for_each(oss_array.begin(), oss_array.end(), [this, index = 0](const std::string &e) mutable {
                        std::array<char, 5> temp_array{}; // Initialize an array of chars with size 5
                        std::copy(e.begin(), e.begin() + std::min(e.size(), temp_array.size()), temp_array.begin());
                        std::copy(temp_array.begin(), temp_array.end(), all[index]->begin());
                        index++;
                    });

                    std::array<uint8_t, 12> config_data_array;
                    std::transform(
                        in_data.begin(),
                        in_data.begin() + config_data_array.size(),
                        config_data_array.begin(), 
                        [](char c) { 
                            return static_cast<uint8_t>(c);
                        }
                    );
                    config = AD5933::Config { config_data_array };

                    std::array<uint8_t, 7> data_data_array;
                    std::transform(
                        in_data.begin() + config_data_array.size(),
                        in_data.end(),
                        data_data_array.begin(), 
                        [](char c) { 
                            return static_cast<uint8_t>(c);
                        }
                    );
                    data = AD5933::DebugData<float> { data_data_array };
                }
            };
        }
    }
}

namespace GUI {
    namespace Windows {
        class Debug {
        public:
            static constexpr std::string_view name_base { "Debug##" };
        private:
            bool first { true };
            size_t index;
            std::string name {  name_base };
            std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
        public:
            enum class Status {
                NotDumped,
                Dumped,
            };
        private:
            Status status { Status::NotDumped };
            Windows::Captures::Debug<5> debug_captures {};
        public:
            Status get_status() const;
            Debug() = default;
            Debug(const size_t index, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm);
            void draw(bool &enable, const ImGuiID side_id, const std::optional<Lock> lock);
        private:
            void draw_input_elements();
            bool dump();
            bool program_and_dump();
            bool command_and_dump(const AD5933::Masks::Or::Ctrl::HB::Command command);
        };
    }
}
