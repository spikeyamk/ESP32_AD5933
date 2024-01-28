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

#include "ad5933/masks/masks.hpp"
#include "ad5933/debug_data/debug_data.hpp"
#include "ad5933/masks/maps.hpp"
#include "ad5933/config/config.hpp"
#include "ad5933/uint_types.hpp"

namespace GUI {
    namespace Windows {
        namespace Captures {
            class Measurement {
            public:
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
                std::array<char, 7> freq_start { freq_start_array_init_lambda(config) };
                std::array<char, 7> freq_inc { freq_inc_array_init_lambda(config) };
                std::array<char, 4> num_of_inc { num_of_inc_array_init_lambda(config)  };
                std::array<char, 4> settling_time_cycles_num { settling_time_cycles_num_array_init_lambda(config) };
                int settling_time_cycles_multiplier_combo = 0;
                int voltage_range_combo = 0;
                int pga_gain_combo = 0;
                int sysclk_src_combo = 0;
                float calibration_impedance = 1000.0f;

                std::array<char, 9> sysclk_freq { sysclk_freq_array_init_lambda(config) };

                Measurement() = default;

                void update_config() {
                    auto stoul_lambda = [](const std::string &num_str) -> std::optional<uint32_t> {
                        try {
                            size_t pos;
                            if(num_str.empty()) {
                                return std::nullopt;
                            }
                            unsigned long number = std::stoul(num_str, &pos);

                            if(pos < std::strlen(num_str.c_str())) {
                                std::cout << "Conversion failed. Not the entire string was converted." << std::endl;
                                return std::nullopt; // Return empty optional if conversion is incomplete
                            } else {
                                if(number > std::numeric_limits<uint32_t>::max()) {
                                    return std::nullopt;
                                } else {
                                    return static_cast<uint32_t>(number);
                                }
                            }
                        } catch (const std::out_of_range& e) {
                            std::cout << "Exception caught: " << e.what() << std::endl;
                            // Handle the out_of_range exception here
                        } catch (const std::invalid_argument& e) {
                            std::cout << "Invalid argument exception caught: " << e.what() << std::endl;
                            // Handle the invalid_argument exception here
                        }
                        return std::nullopt; // Return empty optional in case of exceptions
                    };
        
                    config = AD5933::Config {
                        config.get_command(),
                        std::next(AD5933::Masks::Or::Ctrl::HB::voltage_map.begin(), voltage_range_combo)->first,
                        std::next(AD5933::Masks::Or::Ctrl::HB::pga_gain_map.begin(), pga_gain_combo)->first,
                        std::next(AD5933::Masks::Or::Ctrl::LB::sysclk_src_map.begin(), sysclk_src_combo)->first,
                        [this, &stoul_lambda]() {
                            const auto ret = stoul_lambda(std::string(this->freq_start.begin(), this->freq_start.end()));
                            if(ret.has_value()) {
                                return AD5933::uint_startfreq_t { ret.value() };
                            } else {
                                return this->config.get_start_freq();
                            }
                        }(),
                        [this, &stoul_lambda]() {
                            const auto ret = stoul_lambda(std::string(this->freq_inc.begin(), this->freq_inc.end()));
                            if(ret.has_value()) {
                                return AD5933::uint_incfreq_t { ret.value() };
                            } else {
                                return this->config.get_inc_freq();
                            }
                        }(),
                        [this, &stoul_lambda]() {
                            const auto ret = stoul_lambda(std::string(this->num_of_inc.begin(), this->num_of_inc.end()));
                            if(ret.has_value()) {
                                return AD5933::uint9_t { static_cast<uint16_t>(ret.value()) };
                            } else {
                                return this->config.get_num_of_inc();
                            }
                        }(),
                        [this, &stoul_lambda]() {
                            const auto ret = stoul_lambda(std::string(this->settling_time_cycles_num.begin(), this->settling_time_cycles_num.end()));
                            if(ret.has_value()) {
                                return AD5933::uint9_t { static_cast<uint16_t>(ret.value()) };
                            } else {
                                return this->config.get_settling_time_cycles_number();
                            }
                        }(),
                        std::next(AD5933::Masks::Or::SettlingTimeCyclesHB::multiplier_map.begin(), settling_time_cycles_multiplier_combo)->first,
                    };
                    sysclk_freq = sysclk_freq_array_init_lambda(config);
                }
            private:
                static constexpr auto freq_start_array_init_lambda = [](AD5933::Config &config) -> std::array<char, 7> {
                    std::array<char, 7> ret_val { 0x00 };
                    const auto ret_str = std::to_string(static_cast<uint32_t>(config.get_start_freq().unwrap()));
                    std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
                    return ret_val;
                };

                static constexpr auto freq_inc_array_init_lambda = [](AD5933::Config &config) -> std::array<char, 7> {
                    std::array<char, 7> ret_val { 0x00 };
                    const auto ret_str = std::to_string(static_cast<uint32_t>(config.get_inc_freq().unwrap()));
                    std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
                    return ret_val;
                };

                static constexpr auto num_of_inc_array_init_lambda = [](AD5933::Config &config) -> std::array<char, 4> {
                    std::array<char, 4> ret_val { 0x00 };
                    const auto ret_str = std::to_string(config.get_num_of_inc().unwrap());
                    std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
                    return ret_val;
                };

                static constexpr auto settling_time_cycles_num_array_init_lambda = [](AD5933::Config &config) -> std::array<char, 4> {
                    std::array<char, 4> ret_val { 0x00 };
                    const auto ret_str = std::to_string(config.get_settling_time_cycles_number().unwrap());
                    std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
                    return ret_val;
                };

                static constexpr auto sysclk_freq_array_init_lambda = [](AD5933::Config &config) -> std::array<char, 9> {
                    std::array<char, 9> ret_val { 0x00 };
                    const auto ret_str = std::to_string(static_cast<uint32_t>(config.get_active_sysclk_freq()));
                    std::copy(ret_str.begin(), ret_str.end(), ret_val.begin());
                    return ret_val;
                };
            };

            template<size_t N>
            class DebugReadWriteRegisters {
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

                DebugReadWriteRegisters() = default;

                DebugReadWriteRegisters(const DebugReadWriteRegisters &other) {
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

            class HexDebugReadWriteRegisters : public DebugReadWriteRegisters<5> {
            public:
                HexDebugReadWriteRegisters() :
                    DebugReadWriteRegisters {}
                {}
            };

            class BinDebugReadWriteRegistersCaptures : public DebugReadWriteRegisters<12> {

            };
        }
    }
}