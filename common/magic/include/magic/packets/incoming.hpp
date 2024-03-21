#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <algorithm>

#include "magic/common.hpp"

namespace Magic {
    template<typename T_Variant, typename T_Map>
    class InComingPacket {
        T_MaxPacket raw_data;
    public:
        inline constexpr InComingPacket(const T_MaxPacket& raw_data) :
            raw_data { raw_data }
        {}

        inline constexpr std::optional<T_Variant> to_event_variant() const {
            auto find_it = std::find_if(T_Map::map.begin(), T_Map::map.end(), [&](const auto& e) {
                bool has_header = false;
                std::visit([&](auto&& event) {
                    has_header = static_cast<uint8_t>(event.header) == raw_data[0];
                }, e);
                return has_header;
            });

            if(find_it == T_Map::map.end()) {
                return std::nullopt;
            }

            T_Variant ret;
            std::visit([&](auto&& event) {
                decltype(event.to_raw_data()) tmp_raw_data {};
                std::copy(raw_data.begin(), raw_data.begin() + tmp_raw_data.size(), tmp_raw_data.begin());
                ret = event.from_raw_data(tmp_raw_data);
            }, *find_it);

            return ret;
        }
    };
}