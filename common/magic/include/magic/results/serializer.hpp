#pragma once

#include <serde/serializer.hpp>

#include "magic/results/results.hpp"

namespace Magic {
    namespace Results {
        using Serializer = Pack::apply_to<Serde::Serializer>;
    }
}