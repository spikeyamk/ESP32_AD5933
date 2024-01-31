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

    void to_json(json& j, const SHM& p) {
        j = json{
            {"SHM", p.channels},
        };
    }

    void from_json(const json& j, SHM& p) {
        j.at("SHM").get_to(p.channels);
    }
}