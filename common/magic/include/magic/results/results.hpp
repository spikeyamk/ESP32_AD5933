#pragma once

#include <chrono>
#include <algorithm>

#include <serde/common.hpp>

#include "magic/common.hpp"
#include "magic/misc/gettimeofday.hpp"

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

            namespace Record {
                const std::array<uint8_t, 128> file_header {
                    0x51, 0x46, 0x52, 0xb5, 0x05, 0x99, 0x3b, 0x98,
                    0x01, 0xf6, 0x13, 0xbc, 0xd9, 0xc8, 0xa5, 0xfa,
                    0x67, 0x11, 0x4e, 0x9f, 0x13, 0xce, 0xe4, 0xff,
                    0xe6, 0x78, 0x8a, 0xd7, 0xb2, 0x00, 0xf3, 0xf0,
                    0xff, 0x9f, 0xd5, 0xcf, 0xc3, 0x94, 0x38, 0x80,
                    0xa1, 0xc9, 0x55, 0x6d, 0x54, 0x02, 0x8c, 0xf8,
                    0x15, 0x61, 0x24, 0x1d, 0x6e, 0x1e, 0xaa, 0xc9,
                    0xb4, 0x67, 0x84, 0x01, 0x7a, 0x3f, 0x39, 0xb6,
                    0xd5, 0x34, 0xa6, 0x26, 0x23, 0xc2, 0x77, 0x0f,
                    0x63, 0x83, 0x10, 0x26, 0x9c, 0x7e, 0x5d, 0x8e,
                    0xcd, 0x80, 0x0a, 0xc0, 0x2f, 0xf3, 0x85, 0xf0,
                    0xe7, 0xd3, 0x6b, 0xa3, 0xb4, 0xf3, 0x7b, 0xda,
                    0x59, 0x85, 0x6b, 0xa0, 0x73, 0x4b, 0xf2, 0x0b,
                    0xb3, 0x79, 0x4e, 0xac, 0xa7, 0xe9, 0x48, 0x97,
                    0x6f, 0x78, 0x10, 0xae, 0x3c, 0x98, 0x56, 0x70,
                    0xfb, 0x0e, 0x00, 0x2b, 0x5b, 0x16, 0x93, 0x50,
                };

                struct Entry {
                    double unix_timestamp;
                    float impedance;
                    float phase;

                    Entry() = default;

                    inline constexpr Entry(const double unix_timestamp, const float impedance, float phase) :
                        unix_timestamp{ unix_timestamp },
                        impedance{ impedance },
                        phase{ phase }
                    {}

                    inline Entry(const float impedance, float phase) :
                        unix_timestamp{ []() {
							const auto now { std::chrono::high_resolution_clock::now() };
							const auto since_epoch { now.time_since_epoch() };
                            const auto sec { std::chrono::duration_cast<std::chrono::seconds>(since_epoch) };
							const double tv_sec { static_cast<double>(sec.count()) };
							const double tv_usec { static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count() - std::chrono::duration_cast<std::chrono::microseconds>(sec).count()) };
                            return tv_sec + (tv_usec / 1'000'000.0);
                        }() },
                        impedance{ impedance },
                        phase{ phase }
                    {}

                    using T_RawData = std::array<uint8_t, sizeof(unix_timestamp) + sizeof(impedance) + sizeof(phase)>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret {};

                        const int64_t* tmp_unix_timestamp = static_cast<const int64_t*>(static_cast<const void*>(&unix_timestamp));
                        std::generate(ret.begin(), ret.begin() + sizeof(unix_timestamp), [index = 0, tmp_unix_timestamp]() mutable {
                            return static_cast<uint8_t>(*tmp_unix_timestamp >> (8 * index++)) & 0xFF;
                        });

                        const int32_t* tmp_impedance = static_cast<const int32_t*>(static_cast<const void*>(&impedance));
                        std::generate(ret.begin() + sizeof(unix_timestamp), ret.begin() + sizeof(unix_timestamp) + sizeof(impedance), [index = 0, tmp_impedance]() mutable {
                            return static_cast<uint8_t>(*tmp_impedance >> (8 * index++)) & 0xFF;
                        });

                        const int32_t* tmp_phase = static_cast<const int32_t*>(static_cast<const void*>(&phase));
                        std::generate(ret.begin() + sizeof(unix_timestamp) + sizeof(impedance), ret.begin() + sizeof(unix_timestamp) + sizeof(impedance) + sizeof(phase), [index = 0, tmp_phase]() mutable {
                            return static_cast<uint8_t>(*tmp_phase >> (8 * index++)) & 0xFF;
                        });

                        return ret;
                    }

                    static inline constexpr Entry from_raw_data(const T_RawData& raw_data) {
                        const double* tmp_unix_timestamp = static_cast<const double*>(static_cast<const void*>(raw_data.data()));
                        const float* tmp_impedance = static_cast<const float*>(static_cast<const void*>(raw_data.data() + sizeof(unix_timestamp)));
                        const float* tmp_phase = static_cast<const float*>(static_cast<const void*>(raw_data.data() + sizeof(unix_timestamp) + sizeof(impedance)));
                        return Entry {
                            *tmp_unix_timestamp,
                            *tmp_impedance,
                            *tmp_phase,
                        };
                    }
                };
            }
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