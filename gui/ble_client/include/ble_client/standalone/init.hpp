#pragma once

#include <optional>
#include <simpleble/SimpleBLE.h>

namespace BLE_Client {
    std::optional<SimpleBLE::Adapter> find_default_active_adapter();
}
