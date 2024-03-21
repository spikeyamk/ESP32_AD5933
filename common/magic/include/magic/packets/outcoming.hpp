#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <type_traits>

#include "magic/common.hpp"

namespace Magic {
    template<typename T_Event, size_t N>
    class T_OutComingPacket {
    private:
        std::array<uint8_t, N> raw_data;
    public:
        inline constexpr T_OutComingPacket(const T_Event& event) :
            raw_data { event.to_raw_data() }
        {
            static_assert(N <= Magic::MTU);
        }

        inline constexpr std::array<uint8_t, N> get_raw_data() const {
            return raw_data;
        }

        inline constexpr T_Event to_event() const {
            return T_Event::from_raw_data(raw_data);
        }
    };

    /* Ugly hack lets us use empty structs as inputs into OutComingPacket because sizeof(EmptyStruct) is 1, but we need the sizeof(EmptyStruct) to be zero (it's complicated) */
    template <typename T_Event>
    using OutComingPacket = T_OutComingPacket<T_Event, std::is_empty_v<T_Event> ? sizeof(T_Event) : sizeof(T_Event) + 1>;
}