#pragma once

#include <string_view>
#include <array>

namespace Legal {
    struct License {
        const std::string_view name;
        const std::string_view text;
    };
    extern const std::array<const License, 13> licenses;
}