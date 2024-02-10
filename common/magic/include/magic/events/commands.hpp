#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <variant>
#include <bitset>

#include "magic/misc/gettimeofday.hpp"
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

                /* Auto Commands */
                AutoSave,
                AutoSend,
                AutoEnd,
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
                struct Size {
                    static constexpr Header header { Header::FileSize };
                    T_MaxDataSlice path { 0 };
                    using T_RawData = T_MaxPacket;
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret { static_cast<uint8_t>(header) };
                        std::copy(path.begin(), path.end(), ret.begin() + 1);
                        return ret;
                    }
                    static inline constexpr Size from_raw_data(const T_RawData& raw_data) {
                        decltype(path) tmp;
                        std::copy(raw_data.begin() + 1, raw_data.end(), tmp.begin());
                        return Size { tmp };
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

                    static inline constexpr UpdateTimeval from_raw_data(const T_RawData& raw_data) {
                        const decltype(mytimeval64_t::tv_sec)* tmp_sec = static_cast<const decltype(mytimeval64_t::tv_sec)*>(static_cast<const void*>(raw_data.data() + 1));
                        const decltype(mytimeval64_t::tv_usec)* tmp_usec = static_cast<const decltype(mytimeval64_t::tv_usec)*>(static_cast<const void*>(raw_data.data() + 1 + sizeof(decltype(mytimeval64_t::tv_sec))));
                        return UpdateTimeval {
                            mytimeval64_t { *tmp_sec, *tmp_usec }
                        };
                    }
                };

                struct UpdateTimezone {
                    static constexpr Header header { Header::TimeUpdateTimezone };
                    using T_RawData = std::array<uint8_t, 1 + sizeof(timezone)>;
                    struct timezone tz { 0, 0 };
                    inline constexpr T_RawData to_raw_data() const {
                        T_RawData ret {
                            static_cast<uint8_t>(header),
                        };

                        std::generate(ret.begin() + 1, ret.begin() + 1 + sizeof(decltype(timezone::tz_minuteswest)), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((tz.tz_minuteswest >> (index++ * 8)) & 0xFF);
                        });

                        std::generate(ret.begin() + 1 + sizeof(decltype(timezone::tz_minuteswest)), ret.end(), [index = 0, this]() mutable {
                            return static_cast<uint8_t>((tz.tz_dsttime >> (index++ * 8)) & 0xFF);
                        });

                        return ret;
                    }
                    static inline constexpr UpdateTimezone from_raw_data(const T_RawData& raw_data) {
                        const decltype(timezone::tz_minuteswest)* tmp_minutewest = static_cast<const decltype(timezone::tz_minuteswest)*>(static_cast<const void*>(raw_data.data() + 1));
                        const decltype(timezone::tz_dsttime)* tmp_dsttime = static_cast<const decltype(timezone::tz_dsttime)*>(static_cast<const void*>(raw_data.data() + 1 + sizeof(decltype(timezone::tz_minuteswest))));
                        return UpdateTimezone {
                            *tmp_minutewest, *tmp_dsttime
                        };
                    }
                };
            }
            
            namespace Auto {
                struct Save {
                    static constexpr Header header { Header::AutoSave };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return std::array<uint8_t, 1> { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr Save from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return Save {};
                    }
                };

                struct Send {
                    static constexpr Header header { Header::AutoSend };
                    using T_RawData = std::array<uint8_t, 1>;
                    inline constexpr T_RawData to_raw_data() const {
                        return std::array<uint8_t, 1> { static_cast<uint8_t>(header) };
                    }
                    static inline constexpr Send from_raw_data(const T_RawData& raw_data) {
                        (void)raw_data;
                        return Send {};
                    }
                };

                struct End {
                    static constexpr Header header { Header::AutoEnd };
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
                File::Size,
                File::Remove,
                File::Download,
                File::Upload,
                File::End,

                /* Time Commands */
                Time::UpdateTimeval,
                Time::UpdateTimezone,

                /* Auto Commands */
                Auto::Save,
                Auto::Send,
                Auto::End
            >;

            struct Map {
                static constexpr std::array<Variant, 22> map {
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
                    File::Size{},
                    File::Remove{},
                    File::Download{},
                    File::Upload{},
                    File::End{},

                    /* Time Commands */
                    Time::UpdateTimeval{},
                    Time::UpdateTimezone{},

                    /* Auto Commands */
                    Auto::Save{},
                    Auto::Send{},
                    Auto::End{},
                };
            };
        }
    }
}