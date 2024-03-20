#pragma once

#include <serde/deserializer.hpp>

#include "magic/results/results.hpp"

namespace Magic {
    namespace Results {
        using Deserializer = Pack::apply_to<Serde::Deserializer>;
    }
}