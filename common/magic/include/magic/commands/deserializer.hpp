#pragma once

#include <serde/deserializer.hpp>

#include "magic/commands/commands.hpp"

namespace Magic {
    namespace Commands {
        using Deserializer = Pack::apply_to<Serde::Deserializer>;
    }
}