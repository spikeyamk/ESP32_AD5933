#include "json/shm.hpp"

namespace ns {
    void to_json(json& j, const Channel& p) {
        j = json{
            {"mutex", p.mutex},
            {"condition", p.condition},
        };
    }

    void from_json(const json& j, Channel& p) {
        j.at("mutex").get_to(p.mutex);
        j.at("condition").get_to(p.condition);
    }

    void to_json(json& j, const Channels& p) {
        j = json{ "channels", p.channels_vector };
    }

    void from_json(const json& j, Channels& p) {
        j.at("channels").get_to(p.channels_vector);
    }

    void to_json(json& j, const SHM& p) {
        j = json{ "shm", { p.channels } };
    }

    void from_json(const json& j, SHM& p) {
        j.at("shm").get_to(p.channels);
    }
}