#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <variant>

#include "magic/events/common.hpp"

namespace Magic {
    namespace Events {
        namespace Commands {
            enum class Header {
                /* Debug Commands */
                DebugStart,
                DebugDump,
                DebugProgram,
                DebugCtrlHB,
                DebugEnd,

                /* Sweep Commands */
                SweepConfigure,
                SweepRun,
                SweepEnd,

                /* File Commands */
                FileStart,
                FileFree,
                FileListCount,
                FileList,
                FileSize,
                FileRemove,
                FileDownload,
                FileUpload,
                FileEnd,
            };

            namespace Debug {
                struct Start {
                    static constexpr Header header { Header::DebugStart };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr Start from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return Start {};
                    }
                };
                struct Dump {
                    static constexpr Header header { Header::DebugDump };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr Dump from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return Dump {};
                    }
                };
                struct Program {
                    static constexpr Header header { Header::DebugProgram };
                    std::array<uint8_t, 12> registers_data { 0 };
                    using T_RawData = std::array<uint8_t, 13>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header), };
                        std::copy(registers_data.begin(), registers_data.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr Program from_raw_data(const T_RawData& raw_data) {
                        decltype(registers_data) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return Program { tmp };
                    }
                };
                struct CtrlHB {
                    static constexpr Header header { Header::DebugCtrlHB };
                    uint8_t register_data;
                    using T_RawData = std::array<uint8_t, 2>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header), register_data };
                        return ret;
                    }
                    static inline constexpr CtrlHB from_raw_data(const T_RawData& raw_data) {
                        return CtrlHB { raw_data[1] };
                    }
                };
                struct End {
                    static constexpr Header header { Header::DebugEnd };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr End from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return End {};
                    }
                };
            }

            namespace Sweep {
               struct Configure {
                    static constexpr Header header { Header::SweepConfigure };
                    std::array<uint8_t, 12> registers_data { 0 };
                    using T_RawData = std::array<uint8_t, 13>;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header), };
                        std::copy(registers_data.begin(), registers_data.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr Configure from_raw_data(const T_RawData& raw_data) {
                        decltype(registers_data) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return Configure { tmp };
                    }
                };
                struct Run {
                    static constexpr Header header { Header::SweepRun };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr Run from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return Run {};
                    }
                };
                struct End {
                    static constexpr Header header { Header::SweepEnd };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr End from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return End {};
                    }
                };
            }
            
            namespace File {
                struct Start {
                    static constexpr Header header { Header::FileStart };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return std::array<uint8_t, 1> { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr Start from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return Start {};
                    }
                };
                struct Free {
                    static constexpr Header header { Header::FileFree };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr Free from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return Free {};
                    }
                };
                struct ListCount {
                    static constexpr Header header { Header::FileListCount };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr ListCount from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return ListCount {};
                    }
                };
                struct List {
                    static constexpr Header header { Header::FileList };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr List from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return List {};
                    }
                };
                struct FileSize {
                    static constexpr Header header { Header::FileSize };
                    T_MaxDataSlice path { 0 };
                    using T_RawData = T_MaxPacket;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::copy(path.begin(), path.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr FileSize from_raw_data(const T_RawData& raw_data) {
                        decltype(path) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return FileSize { tmp };
                    }
                };
                struct Remove {
                    static constexpr Header header { Header::FileRemove };
                    T_MaxDataSlice path { 0 };
                    using T_RawData = T_MaxPacket;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::copy(path.begin(), path.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr Remove from_raw_data(const T_RawData& raw_data) {
                        decltype(path) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return Remove { tmp };
                    }
                };
                struct Download {
                    static constexpr Header header { Header::FileDownload };
                    T_MaxDataSlice path { 0 };
                    using T_RawData = T_MaxPacket;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::copy(path.begin(), path.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr Download from_raw_data(const T_RawData& raw_data) {
                        decltype(path) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return Download { tmp };
                    }
                };
                struct Upload {
                    static constexpr Header header { Header::FileUpload };
                    T_MaxDataSlice path { 0 };
                    using T_RawData = T_MaxPacket;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::copy(path.begin(), path.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr Download from_raw_data(const T_RawData& raw_data) {
                        decltype(path) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return Download { tmp };
                    }
                };
                struct End {
                    static constexpr Header header { Header::FileEnd };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr End from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return End {};
                    }
                };
            }

            using Variant = std::variant<
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
                File::FileSize,
                File::Remove,
                File::Download,
                File::Upload,
                File::End
            >;

            struct Map {
                static constexpr std::array<Variant, 17> map {
                    /* Debug Commands */
                    Debug::Start{},
                    Debug::Dump{},
                    Debug::Program{},
                    Debug::CtrlHB{},
                    Debug::End{},

                    /* Sweep Commands */
                    Sweep::Configure{},
                    Sweep::Run{},
                    Sweep::End{},

                    /* File Commands */
                    File::Start{},
                    File::Free{},
                    File::ListCount{},
                    File::List{},
                    File::FileSize{},
                    File::Remove{},
                    File::Download{},
                    File::Upload{},
                    File::End{}
                };
            };
        }
    }
}