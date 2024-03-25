#pragma once

#include <serde/common.hpp>

#include "magic/misc/gettimeofday.hpp"
#include "magic/common.hpp"

namespace Magic {
    namespace Commands {
        namespace Debug {
            struct Start {};
            struct Dump {};
            struct Program {
                std::array<uint8_t, 12> registers_data { 0 };
            };
            struct CtrlHB {
                uint8_t register_data;
            };
            struct End {};
        }

        namespace Sweep {
            struct Configure {
                std::array<uint8_t, 12> registers_data { 0 };
            };
            struct Run {};
            struct End {};
        }

        namespace File {
            struct Start {};
            struct Free {};
            struct ListCount {};
            struct List {};
            struct Size {
                T_MaxDataSlice path { 0 };
            };
            struct Remove {
                T_MaxDataSlice path { 0 };
            };
            struct Download {
                T_MaxDataSlice path { 0 };
            };
            struct Format {};
            struct CreateTestFiles {};
            struct End {};
        }

        namespace Time {
            using UpdateTimeval = mytimeval64_t;
            using UpdateTimezone = struct timezone;
        }

        namespace Auto {
            struct Save {
                uint16_t tick_ms;
            };

            struct Send {
                uint16_t tick_ms;
            };

            struct End {};
        }

        using Pack = Serde::pack_holder<
            /* Debug Commands */
            Debug::Start,
            Debug::Dump,
            Debug::Program,
            Debug::CtrlHB,
            Debug::End,

            /* Sweep Commands */
            Sweep::Configure,
            Sweep::Run,
            Sweep::End,

            /* File Commands */
            File::Start,
            File::Free,
            File::ListCount,
            File::List,
            File::Size,
            File::Remove,
            File::Download,
            File::Format,
            File::CreateTestFiles,
            File::End,

            /* Time Commands */
            Time::UpdateTimeval,
            Time::UpdateTimezone,

            /* Auto Commands */
            Auto::Save,
            Auto::Send,
            Auto::End
        >;
    }
}