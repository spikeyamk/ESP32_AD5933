#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <variant>

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
                        const decltype(bytes_free)* tmp_bytes_free = reinterpret_cast<const decltype(bytes_free)*>(raw_data.data() + 1);
                        const decltype(bytes_total)* tmp_bytes_total = reinterpret_cast<const decltype(bytes_total)*>(raw_data.data() + 1 + sizeof(bytes_free));
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
                        const decltype(num_of_files)* tmp_num_of_files = reinterpret_cast<const decltype(num_of_files)*>(raw_data.data() + 1);
                        return ListCount { *tmp_num_of_files };
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
                        const decltype(num_of_bytes)* tmp_num_of_bytes = reinterpret_cast<const decltype(num_of_bytes)*>(raw_data.data() + 1);
                        return Size { *tmp_num_of_bytes };
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
                File::Upload
            >;

            struct Map {
                static constexpr std::array<Variant, 8> map {
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
                    File::Upload{}
                };
            };
        }
    }
}