#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <variant>

#include "magic/misc/gettimeofday.hpp"
#include "magic/events/common.hpp"

namespace Magic {
    namespace Events {
        namespace Results {
            enum class Header {
                /* Debug  */
                DebugDump,

                /* Sweep Results */
                SweepValidData,

                /* File Results */
                FileFree,
                FileListCount,
                FileList,
                FileSize,
                FileDownload,
                FileUpload,

                /* Auto Results */
                AutoTimeval,
                AutoPoint,
            };

            namespace Debug {
                struct Dump {
                    static constexpr Header header { Header::DebugDump };
                    std::array<uint8_t, 19> registers_data { 0 };
                    using T_RawData = T_MaxPacket;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header), };
                        std::copy(registers_data.begin(), registers_data.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr Dump from_raw_data(const T_RawData& raw_data) {
                        decltype(registers_data) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return Dump { tmp };
                    }
                };
            }

            namespace Sweep {
                struct ValidData {
                    static constexpr Header header { Header::SweepValidData };
                    std::array<uint8_t, 4> real_imag_registers_data { 0 };
                    using T_RawData = std::array<uint8_t, 5>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header), };
                        std::copy(real_imag_registers_data.begin(), real_imag_registers_data.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr ValidData from_raw_data(const T_RawData& raw_data) {
                        decltype(real_imag_registers_data) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return ValidData { tmp };
                    }
                };
            }
            
            namespace File {
                struct Free {
                    static constexpr Header header { Header::FileFree };
                    uint64_t bytes_free;
                    uint64_t bytes_total;
                    using T_RawData = std::array<uint8_t, 1 + sizeof(bytes_free) + sizeof(bytes_total)>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::generate(ret.begin() + 1, ret.begin() + 1 + sizeof(bytes_free), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((bytes_free >> (8 * index++)) & 0xFF);
                        });
                        std::generate(ret.begin() + 1 + sizeof(bytes_free), ret.end(), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((bytes_total >> (8 * index++)) & 0xFF);
                        });
                        return ret;
                    }
                    static inline constexpr Free from_raw_data(const T_RawData& raw_data) {
                        const decltype(bytes_free)* tmp_bytes_free = static_cast<const decltype(bytes_free)*>(static_cast<const void*>(raw_data.data() + 1));
                        const decltype(bytes_total)* tmp_bytes_total = static_cast<const decltype(bytes_total)*>(static_cast<const void*>(raw_data.data() + 1 + sizeof(bytes_free)));
                        return Free { 
                            .bytes_free = *tmp_bytes_free, 
                            .bytes_total = *tmp_bytes_total,
                        };
                    }
                };

                struct ListCount {
                    static constexpr Header header { Header::FileListCount };
                    uint64_t num_of_files;
                    using T_RawData = std::array<uint8_t, 1 + sizeof(num_of_files)>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::generate(ret.begin() + 1, ret.end(), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((num_of_files >> (8 * index++)) & 0xFF);
                        });
                        return ret;
                    }
                    static inline constexpr ListCount from_raw_data(const T_RawData& raw_data) {
                        const decltype(num_of_files)* tmp_num_of_files = static_cast<const decltype(num_of_files)*>(static_cast<const void*>(raw_data.data() + 1));
                        return ListCount {
                            *tmp_num_of_files
                        };
                    }
                };

                struct List {
                    static constexpr Header header { Header::FileList };
                    T_MaxDataSlice path { 0 };
                    using T_RawData = T_MaxPacket;
                    inline constexpr T_MaxPacket to_raw_data() const {
                        T_MaxPacket ret { static_cast<uint8_t>(header) };
                        std::copy(path.begin(), path.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr List from_raw_data(const T_RawData& raw_data) {
                        decltype(path) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return List{ tmp };
                    }
                };

                struct Size {
                    static constexpr Header header { Header::FileSize };
                    uint64_t num_of_bytes;
                    using T_RawData = std::array<uint8_t, 1 + sizeof(num_of_bytes)>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::generate(ret.begin() + 1, ret.end(), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((num_of_bytes >> (8 * index++)) & 0xFF);
                        });
                        return ret;
                    }
                    static inline constexpr Size from_raw_data(const T_RawData& raw_data) {
                        const decltype(num_of_bytes)* tmp_num_of_bytes = static_cast<const decltype(num_of_bytes)*>(static_cast<const void*>(raw_data.data() + 1));
                        return Size {
                            *tmp_num_of_bytes
                        };
                    }
                };

                struct Download {
                    static constexpr Header header { Header::FileDownload };
                    T_MaxDataSlice slice { 0 };
                    using T_RawData = T_MaxPacket;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::copy(slice.begin(), slice.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr Download from_raw_data(const T_RawData& raw_data) {
                        decltype(slice) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return Download { tmp };
                    }
                };

                struct Upload {
                    static constexpr Header header { Header::FileUpload };
                    T_MaxDataSlice slice { 0 };
                    using T_RawData = T_MaxPacket;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::copy(slice.begin(), slice.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr Upload from_raw_data(const T_RawData& raw_data) {
                        decltype(slice) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return Upload { tmp };
                    }
                };
            }

            namespace Auto {
                struct Timeval {
                    static constexpr Header header { Header::AutoTimeval };
                    using T_RawData = std::array<uint8_t, 1 + sizeof(mytimeval64_t)>;
                    mytimeval64_t tv;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret {
                            static_cast<uint8_t>(header),
                        };

                        std::generate(ret.begin() + 1, ret.begin() + sizeof(decltype(mytimeval64_t::tv_sec)), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((tv.tv_sec >> (index++ * 8)) & 0xFF);
                        });

                        std::generate(ret.begin() + sizeof(decltype(mytimeval64_t::tv_sec)), ret.end(), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((tv.tv_usec >> (index++ * 8)) & 0xFF);
                        });

                        return ret;
                    }

                    static inline constexpr Timeval from_raw_data(const T_RawData& raw_data) {
                        const decltype(mytimeval64_t::tv_sec)* tmp_sec = static_cast<const decltype(mytimeval64_t::tv_sec)*>(static_cast<const void*>(raw_data.data() + 1));
                        const decltype(mytimeval64_t::tv_usec)* tmp_usec = static_cast<const decltype(mytimeval64_t::tv_usec)*>(static_cast<const void*>(raw_data.data() + 1 + sizeof(decltype(mytimeval64_t::tv_sec))));
                        return Timeval {
                            mytimeval64_t { *tmp_sec, *tmp_usec }
                        };
                    }



                };

                struct Point {
                    static constexpr Header header { Header::AutoPoint };
                    enum class Status {
                        Open,
                        Short,
                        Valid,
                    };
                    enum class Config {
                        Default,
                    };
                    uint8_t status;
                    uint8_t config;
                    float impedance;
                    float phase;
                    using T_RawData = std::array<uint8_t, 1 + sizeof(status) + sizeof(config) + sizeof(impedance) + sizeof(phase)>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret {
                            static_cast<uint8_t>(header),
                            static_cast<uint8_t>(status),
                            static_cast<uint8_t>(config),
                        };

                        const int32_t* tmp_impedance = static_cast<const int32_t*>(static_cast<const void*>(&impedance));
                        std::generate(ret.begin() + 3, ret.begin() + 3 + sizeof(impedance), [index = 0, tmp_impedance]() mutable {
                            return static_cast<uint8_t>(*tmp_impedance >> (8 * index++)) & 0xFF;
                        });

                        const int32_t* tmp_phase = static_cast<const int32_t*>(static_cast<const void*>(&phase));
                        std::generate(ret.begin() + 3 + sizeof(impedance), ret.begin() + 3 + sizeof(impedance) + sizeof(phase), [index = 0, tmp_phase]() mutable {
                            return static_cast<uint8_t>(*tmp_phase >> (8 * index++)) & 0xFF;
                        });

                        return ret;
                    }
                    static inline constexpr Point from_raw_data(const T_RawData& raw_data) {
                        const uint8_t tmp_status = raw_data[1];
                        const uint8_t tmp_config = raw_data[2];
                        const float* tmp_impedance = static_cast<const float*>(static_cast<const void*>(raw_data.data() + 3));
                        const float* tmp_phase = static_cast<const float*>(static_cast<const void*>(raw_data.data() + 3 + sizeof(impedance)));
                        return Point {
                            .status = tmp_status,
                            .config = tmp_config,
                            .impedance = *tmp_impedance,
                            .phase = *tmp_phase,
                        };
                    }
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

                    static constexpr size_t size {
                        sizeof(Magic::Events::Results::Auto::Point::T_RawData)
                        + sizeof(Magic::Events::Results::Auto::Timeval::T_RawData)
                    };
                }
            }
        
            using Variant = std::variant<
                /* Debug Results */
                Debug::Dump,

                /* Sweep Results */
                Sweep::ValidData,

                /* File Results */
                File::Free,
                File::ListCount,
                File::List,
                File::Size,
                File::Download,
                File::Upload,

                /* Auto Results */
                Auto::Timeval,
                Auto::Point
            >;

            struct Map {
                static constexpr std::array<Variant, 10> map {
                    /* Debug Results */
                    Debug::Dump{},

                    /* Sweep Results */
                    Sweep::ValidData{},

                    /* File Results */
                    File::Free{},
                    File::ListCount{},
                    File::List{},
                    File::Size{},
                    File::Download{},
                    File::Upload{},

                    /* Auto Results */
                    Auto::Timeval{},
                    Auto::Point{},
                };
            };
        }
    }
}