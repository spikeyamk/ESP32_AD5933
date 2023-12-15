#pragma once

#include <iostream>
#include <array>
#include <utility>

#include "ad5933/config/masks.hpp"

namespace AD5933 {
    namespace Masks {
        namespace Or {
            namespace Ctrl {
                namespace HB {
                    using CommandPair = std::pair<Masks::Or::Ctrl::HB::Command, const char*>;
                    constexpr std::array<CommandPair, 11> command_map {
                        CommandPair{ Command::Nop_0, "Nop_0" },
                        CommandPair{ Command::InitStartFreq, "InitStartFreq" },
                        CommandPair{ Command::StartFreqSweep, "StartFreqSweep" },
                        CommandPair{ Command::IncFreq, "IncFreq" },
                        CommandPair{ Command::RepeatFreq, "RepeatFreq" },
                        CommandPair{ Command::Nop_1, "Nop_1" },
                        CommandPair{ Command::MeasureTemp, "MeasureTemp" },
                        CommandPair{ Command::PowerDownMode, "PowerDownMode" },
                        CommandPair{ Command::StandbyMode, "StandbyMode" },
                        CommandPair{ Command::Nop_2, "Nop_2" },
                        CommandPair{ Command::Nop_3, "Nop_3" },
                    };

                    using VoltagePair = std::pair<Masks::Or::Ctrl::HB::VoltageRange, const char*>;
                    constexpr std::array<VoltagePair, 4> voltage_map {
                        VoltagePair{ VoltageRange::Two_Vppk, "Two_Vppk" },
                        VoltagePair{ VoltageRange::One_Vppk, "One_Vppk" },
                        VoltagePair{ VoltageRange::FourHundred_mVppk, "FourHundred_mVppk" },
                        VoltagePair{ VoltageRange::TwoHundred_mVppk, "TwoHundred_mVppk" },
                    };

                    using PGA_GainPair = std::pair<Masks::Or::Ctrl::HB::PGA_Gain, const char*>;
                    constexpr std::array<PGA_GainPair, 2> pga_gain_map {
                        PGA_GainPair{ PGA_Gain::OneTime, "OneTime" },
                        PGA_GainPair{ PGA_Gain::FiveTimes, "FiveTimes" },
                    };
                }

                namespace LB {
                    using SysClkSrcPair = std::pair<Masks::Or::Ctrl::LB::SysClkSrc, const char*>;
                    constexpr std::array<SysClkSrcPair, 2> sysclk_src_map {
                        SysClkSrcPair{ SysClkSrc::Internal, "Internal" },
                        SysClkSrcPair{ SysClkSrc::External, "External" },
                    };
                }
            }
            
            namespace SettlingTimeCyclesHB {
                using MultiplierPair = std::pair<Multiplier, const char*>;
                constexpr std::array<MultiplierPair, 4> multiplier_map {
                    MultiplierPair{ Multiplier::OneTime, "OneTime" },
                    MultiplierPair{ Multiplier::TwoTimes, "TwoTimes" },
                    MultiplierPair{ Multiplier::FourTimes, "FourTimes" },
                    MultiplierPair{ Multiplier::Reserved, "Reserved" },
                };
            }
        }
    }
}

std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::Ctrl::HB::Command command);
std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::Ctrl::HB::VoltageRange range);
std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::Ctrl::HB::PGA_Gain pga_gain);
std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::Ctrl::LB::SysClkSrc sysclk_src);
std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::SettlingTimeCyclesHB::Multiplier multiplier);

