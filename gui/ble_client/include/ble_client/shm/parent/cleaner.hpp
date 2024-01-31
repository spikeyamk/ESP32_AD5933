#pragma once

#include <optional>

#include "json/shm.hpp"

namespace BLE_Client {
    namespace SHM {
        class Cleaner {
        private:
            const std::optional<ns::SHM> read_json() const noexcept;
        public:
            ~Cleaner() noexcept;
        };
    }
}