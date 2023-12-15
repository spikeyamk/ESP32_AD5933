#pragma once

#include <cstdint>
#include <bitset>

namespace AD5933 {
    namespace Masks {
        namespace Or {
            namespace Ctrl {
                namespace HB {
                    enum class Command : uint8_t {
                        Nop_0 							= 0b0000'0000u,
                        InitStartFreq                   = 0b0001'0000u,
                        StartFreqSweep 			        = 0b0010'0000u,
                        IncFreq                         = 0b0011'0000u,
                        RepeatFreq 				        = 0b0100'0000u,

                        Nop_1 							= 0b1000'0000u,
                        MeasureTemp 			        = 0b1001'0000u,
                        PowerDownMode			        = 0b1010'0000u,
                        StandbyMode 					= 0b1011'0000u,
                        Nop_2 							= 0b1100'0000u,
                        Nop_3 							= 0b1101'0000u,
                    };

                    enum class VoltageRange : uint8_t {
                        Two_Vppk      		       = 0b0000'0000u,
                        TwoHundred_mVppk           = 0b0000'0010u,
                        FourHundred_mVppk          = 0b0000'0100u,
                        One_Vppk      		       = 0b0000'0110u,
                    };

                    enum class PGA_Gain : uint8_t {
                        OneTime   = 0b0000'0001u,
                        FiveTimes = 0b0000'0000u,
                    };
                }
                namespace LB {
                    enum class SysClkSrc {
                        Internal = 0b0000'0000u,
                        External = 0b0000'1000u,
                    };
                    constexpr uint8_t Reset = 0b0001'0000u;
                }
            }

            namespace SettlingTimeCyclesHB {
                enum class Multiplier : uint8_t {
                    OneTime    = 0b0000'0000u,
                    TwoTimes   = 0b0000'0010u,
                    Reserved   = 0b0000'0100u,
                    FourTimes  = 0b0000'0110u,
                };
            } 
        }
       
        namespace And {
            namespace Ctrl {
                namespace HB {
                    constexpr std::bitset<8> Command { 0b1111'0000u };
                    constexpr std::bitset<8> VoltageRange { 0b0000'0110u };
                    constexpr std::bitset<8> PGA_Gain { 0b0000'0001u };
                }

                namespace LB {
                    constexpr std::bitset<8> SysClkSrc { 0b0000'1000u };
                    constexpr std::bitset<8> Reset { 0b0001'0000u };
                }
            }

            namespace SettlingTimeCyclesHB {
                constexpr std::bitset<8> Multiplier { 0b0000'0110u };
                constexpr std::bitset<8> Number { 0b0000'0001u };
            }

            constexpr std::bitset<8> Status { 0b0000'0111u };
        }
    }
}
 