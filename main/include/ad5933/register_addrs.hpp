#pragma once

namespace AD5933 {
    enum class RegsRW {
        ControlHB = 0x80,
        ControlLB = 0x81,
        FreqStartHB = 0x82,
        FreqStartMB = 0x83,
        FreqStartLB = 0x84,
        FreqIncHB = 0x85,
        FreqIncMB = 0x86,
        FreqIncLB = 0x87,
        IncNumHB = 0x88,
        IncNumLB = 0x89,
        SetCyclesHB = 0x8A,
        SetCyclesLB = 0x8B
    };

    enum class RegsRW_RO {
 		/* Read/Write Register addresses */
        ControlHB = static_cast<uint8_t>(RegsRW::ControlHB),
        ControlLB = static_cast<uint8_t>(RegsRW::ControlLB),
        FreqStartHB = static_cast<uint8_t>(RegsRW::FreqStartHB),
        FreqStartMB = static_cast<uint8_t>(RegsRW::FreqStartMB),
        FreqStartLB = static_cast<uint8_t>(RegsRW::FreqStartLB),
        FreqIncHB = static_cast<uint8_t>(RegsRW::FreqIncHB),
        FreqIncMB = static_cast<uint8_t>(RegsRW::FreqIncMB),
        FreqIncLB = static_cast<uint8_t>(RegsRW::FreqIncLB),
        IncNumHB = static_cast<uint8_t>(RegsRW::IncNumHB),
        IncNumLB = static_cast<uint8_t>(RegsRW::IncNumLB),
        SetCyclesHB = static_cast<uint8_t>(RegsRW::SetCyclesHB),
        SetCyclesLB = static_cast<uint8_t>(RegsRW::SetCyclesLB),

		/* Read-only Register addresses */
        Status = 0x8F,
        TempDataHB = 0x92,
        TempDataLB = 0x93,
        RealDataHB = 0x94,
        RealDataLB = 0x95,
        ImagDataHB = 0x96,
        ImagDataLB = 0x97,
    };
}