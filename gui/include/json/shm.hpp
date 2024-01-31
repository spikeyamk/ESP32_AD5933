#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace ns {
    struct Channel {
        std::string mutex;
        std::string condition;
    };

    void to_json(json& j, const Channel& p);
    void from_json(const json& j, Channel& p);

    struct Channels {
        std::vector<Channel> channels_vector;
    };

    void to_json(json& j, const Channels& p);
    void from_json(const json& j, Channels& p);

    struct SHM {
        Channels channels;
    };

    void to_json(json& j, const SHM& p);
    void from_json(const json& j, SHM& p);
}