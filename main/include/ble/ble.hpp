#pragma once

#include <memory>
#include <cstdint>
#include <atomic>
#include <optional>
#include <thread>
#include <array>

#include "i2c/ad5933.hpp"
#include "magic_packets.hpp"

namespace NimBLE {
    extern uint16_t conn_handle;
    extern uint16_t body_composition_measurement_characteristic_handle;
    extern std::atomic<bool> heartbeat_running;
    extern std::optional<std::thread> heartbeat_thread;
    extern char characteristic_received_value[500];
    extern std::optional<MagicPackets::MagicPacket_T> received_packet;
    void run();
    void advertise();
    void stopBLE();
    void create_heartbeat_task();
    bool notify(const std::array<uint8_t, 20> &message);
}

namespace NimBLE {
    template<size_t n_bytes>
    bool notify_with_footer(const std::array<uint8_t, n_bytes> &message, const MagicPackets::MagicPacket_T &footer) {
        static_assert(n_bytes <= 20, "n_bytes must be less than or equal to 20");
     	auto send_buf = footer;
        std::copy(message.begin(), message.end(), send_buf.begin());
        return notify(send_buf);
    }
}