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
                        std::copy(registers_data.begin() + 1, registers_data.end(), ret.begin() + 1);
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
                        std::copy(real_imag_registers_data.begin() + 1, real_imag_registers_data.end(), ret.begin() + 1);
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
                    uint64_t bytes_used;
                    uint64_t bytes_free;
                    using T_RawData = std::array<uint8_t, 17>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::generate(ret.begin() + 1, ret.end(), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((bytes_used >> (8 * index++)) & 0xFF);
                        });
                        std::generate(ret.begin() + 1 + 8, ret.end(), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((bytes_free >> (8 * index++)) & 0xFF);
                        });
                        return ret;
                    }
                    static inline constexpr Free from_raw_data(const T_RawData& raw_data) {
                        uint64_t bytes_used;
                        std::for_each(raw_data.begin() + 1 + 0, raw_data.end(), [&](const uint8_t e) {
                            bytes_used = (bytes_used << 8) | e;
                        });
                        uint64_t bytes_free;
                        std::for_each(raw_data.begin() + 1 + 8, raw_data.end(), [&](const uint8_t e) {
                            bytes_free = (bytes_free << 8) | e;
                        });
                        return Free { bytes_used, bytes_free };
                    }
                };
                struct ListCount {
                    static constexpr Header header { Header::FileListCount };
                    uint64_t num_of_files;
                    using T_RawData = std::array<uint8_t, 9>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::generate(ret.begin() + 1, ret.end(), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((num_of_files >> (8 * index++)) & 0xFF);
                        });
                        return ret;
                    }
                    static inline constexpr ListCount from_raw_data(const T_RawData& raw_data) {
                        uint64_t num_of_files;
                        std::for_each(raw_data.begin() + 1 + 0, raw_data.end(), [&](const uint8_t e) {
                            num_of_files = (num_of_files << 8) | e;
                        });
                        return ListCount { num_of_files };
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
                struct FileSize {
                    static constexpr Header header { Header::FileSize };
                    uint64_t num_of_bytes;
                    using T_RawData = std::array<uint8_t, 9>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::generate(ret.begin() + 1, ret.end(), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((num_of_bytes >> (8 * index++)) & 0xFF);
                        });
                        return ret;
                    }
                    static inline constexpr FileSize from_raw_data(const T_RawData& raw_data) {
                        uint64_t num_of_bytes;
                        std::for_each(raw_data.begin() + 1 + 0, raw_data.end(), [&](const uint8_t e) {
                            num_of_bytes = (num_of_bytes << 8) | e;
                        });
                        return FileSize { num_of_bytes };
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
                File::FileSize,
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
                    File::FileSize{},
                    File::Download{},
                    File::Upload{}
                };
            };
        }
    }
}