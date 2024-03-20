#pragma once

#include <serde/serializer.hpp>

#include "magic/commands/commands.hpp"

namespace Magic {
    namespace Commands {
        using Serializer = Pack::apply_to<Serde::Serializer>;
    }
}