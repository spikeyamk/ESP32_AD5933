#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <variant>
#include <bitset>

#include <sys/cdefs.h>
#include <sys/time.h>

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

                /* Time Commands */
                TimeUpdateTimeval,
                TimeUpdateTimezone,
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

            namespace Time {
                struct UpdateTimeval {
                    static constexpr Header header { Header::TimeUpdateTimeval };
                    using T_RawData = std::array<uint8_t, 1 + sizeof(timeval)>;
                    timeval tv;
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData {
                            static_cast<uint8_t>(header),
                            static_cast<uint8_t>((tv.tv_sec >> (0 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_sec >> (1 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_sec >> (2 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_sec >> (3 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_sec >> (4 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_sec >> (5 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_sec >> (6 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_sec >> (7 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_usec >> (0 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_usec >> (1 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_usec >> (2 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_usec >> (3 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_usec >> (4 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_usec >> (5 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_usec >> (6 * 8)) & 0xFF),
                            static_cast<uint8_t>((tv.tv_usec >> (7 * 8)) & 0xFF),
                        };
                    }

                    static inline constexpr UpdateTimeval from_raw_data(const T_RawData& raw_data) {
                        const decltype(timeval::tv_sec)* tmp_sec = reinterpret_cast<const decltype(timeval::tv_sec)*>(raw_data.data() + 1);
                        const decltype(timeval::tv_usec)* tmp_usec = reinterpret_cast<const decltype(timeval::tv_usec)*>(raw_data.data() + 1 + sizeof(decltype(timeval::tv_sec)));
                        const timeval tmp_tv { *tmp_sec, *tmp_usec };
                        return UpdateTimeval { tmp_tv };
                    }
                };

                struct UpdateTimezone {
                    static constexpr Header header { Header::TimeUpdateTimezone };
                    using T_RawData = std::array<uint8_t, 1 + sizeof(timezone)>;
                    struct timezone tz { 0, 0 };
                    inline constexpr T_RawData to_raw_data() const {
                        return T_RawData {
                            static_cast<uint8_t>(header),
                            static_cast<uint8_t>((tz.tz_minuteswest >> (0 * 8)) & 0xFF),
                            static_cast<uint8_t>((tz.tz_minuteswest >> (1 * 8)) & 0xFF),
                            static_cast<uint8_t>((tz.tz_minuteswest >> (2 * 8)) & 0xFF),
                            static_cast<uint8_t>((tz.tz_minuteswest >> (3 * 8)) & 0xFF),
                            static_cast<uint8_t>((tz.tz_dsttime >> (0 * 8)) & 0xFF),
                            static_cast<uint8_t>((tz.tz_dsttime >> (1 * 8)) & 0xFF),
                            static_cast<uint8_t>((tz.tz_dsttime >> (2 * 8)) & 0xFF),
                            static_cast<uint8_t>((tz.tz_dsttime >> (3 * 8)) & 0xFF),
                        };
                    }
                    static inline constexpr UpdateTimezone from_raw_data(const T_RawData& raw_data) {
                        const decltype(timezone::tz_minuteswest)* tmp_minutewest = reinterpret_cast<const decltype(timezone::tz_minuteswest)*>(raw_data.data() + 1);
                        const decltype(timezone::tz_dsttime)* tmp_dsttime = reinterpret_cast<const decltype(timezone::tz_dsttime)*>(raw_data.data() + 1 + sizeof(decltype(timezone::tz_minuteswest)));
                        return UpdateTimezone { *tmp_minutewest, *tmp_dsttime };
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
                File::End,

                /* Time Commands */
                Time::UpdateTimeval,
                Time::UpdateTimezone
            >;

            struct Map {
                static constexpr std::array<Variant, 19> map {
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
                    File::End{},

                    /* Time Commands */
                    Time::UpdateTimeval{},
                    Time::UpdateTimezone{},
                };
            };
        }
    }
}