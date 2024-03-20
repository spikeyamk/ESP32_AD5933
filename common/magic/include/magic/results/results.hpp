#pragma once

#include <serde/common.hpp>

#include "magic/common.hpp"

namespace Magic {
    namespace Results {
        namespace Debug {
            struct Dump {
                std::array<uint8_t, 19> registers_data { 0 };
            };
        }

        namespace Sweep {
            struct ValidData {
                std::array<uint8_t, 4> real_imag_registers_data { 0 };
            };
        }
        
        namespace File {
            struct Free {
                uint64_t used_bytes;
                uint64_t total_bytes;
            };

            struct ListCount {
                uint64_t num_of_files;
            };

            struct List {
                T_MaxDataSlice path { 0 };
            };

            struct Size {
                uint64_t num_of_bytes;
            };

            struct Remove {
                bool status;
           };

            struct Format {
                bool status;
            };

            struct Download {
                T_MaxDataSlice slice { 0 };
            };

            struct CreateTestFiles {
                bool status;
            };
        }

        namespace Auto {
            using Timeval = mytimeval64_t;
            struct Point {
                enum class Status {
                    Open,
                    Short,
                    Valid,
                };
                enum class Config {
                    Default,
                };
                Status status;
                Config config;
                float impedance;
                float phase;
            };
        }

        using Pack = Serde::pack_holder<
            /* Debug Results */
            Debug::Dump,

            /* Sweep Results */
            Sweep::ValidData,

            /* File Results */
            File::Free,
            File::ListCount,
            File::List,
            File::Size,
            File::Remove,
            File::Format,
            File::Download,
            File::CreateTestFiles,

            /* Auto Results */
            Auto::Timeval,
            Auto::Point
        >;
    }
}