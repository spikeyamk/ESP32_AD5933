#pragma once

#include <memory>
#include <cstdint>
#include <atomic>
#include <optional>
#include <thread>

#include "ad5933.hpp"

namespace NimBLE {
    extern uint16_t conn_handle;
    extern uint16_t body_composition_measurement_characteristic_handle;
    extern std::atomic<bool> heartbeat_running;
    extern std::optional<std::thread> heartbeat_thread;
    extern char characteristic_received_value[500];
    extern std::optional<std::array<uint8_t, 23>> received_packet;
    void run();
    void advertise();
    void stopBLE();
    void create_heartbeat_task();
}