#pragma once

#include <iostream>
#include <utility>
#include <array>
#include <cstdint>

#include "ad5933/reg_addrs/reg_addrs.hpp"

namespace AD5933 {
    namespace RegAddrs {
        using RW_RO_Pair = std::pair<RW_RO, const char*>;
        constexpr std::array<RW_RO_Pair, 19> reg_map {
            RW_RO_Pair{ RW_RO::ControlHB, "ControlHB" },
            RW_RO_Pair{ RW_RO::ControlLB, "ControlLB" },
            RW_RO_Pair{ RW_RO::FreqStartHB, "FreqStartHB" },
            RW_RO_Pair{ RW_RO::FreqStartMB, "FreqStartMB" },
            RW_RO_Pair{ RW_RO::FreqStartLB, "FreqStartLB" },
            RW_RO_Pair{ RW_RO::FreqIncHB, "FreqIncHB" },
            RW_RO_Pair{ RW_RO::FreqIncMB, "FreqIncMB" },
            RW_RO_Pair{ RW_RO::FreqIncLB, "FreqIncLB" },
            RW_RO_Pair{ RW_RO::IncNumHB, "IncNumHB" },
            RW_RO_Pair{ RW_RO::IncNumLB, "IncNumLB" },
            RW_RO_Pair{ RW_RO::SetCyclesHB, "SetCyclesHB" },
            RW_RO_Pair{ RW_RO::SetCyclesLB, "SetCyclesLB" },

            RW_RO_Pair{ RW_RO::Status, "Status" },

            RW_RO_Pair{ RW_RO::TempDataHB, "TempDataHB" },
            RW_RO_Pair{ RW_RO::TempDataLB, "TempDataLB" },
            RW_RO_Pair{ RW_RO::RealDataHB, "RealDataHB" },
            RW_RO_Pair{ RW_RO::RealDataLB, "RealDataLB" },
            RW_RO_Pair{ RW_RO::ImagDataHB, "ImagDataHB" },
            RW_RO_Pair{ RW_RO::ImagDataLB, "ImagDataLB" },
        };

        template<typename T_Enum>
        const char* get_map_str(const T_Enum e) {
            for(const auto &i: reg_map) {
                if(i.first == RW_RO(static_cast<uint8_t>(e))) {
                    return i.second;
                }
            }
            return nullptr;
        }

        std::ostream& operator<<(std::ostream &os, const RW e);
        std::ostream& operator<<(std::ostream &os, const RW_RO e);
    }
}