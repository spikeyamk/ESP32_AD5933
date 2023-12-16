#pragma once

#include <cstdint>

#include "magic/packets.hpp"

namespace BLE {
    namespace Server {
        void run();
        void advertise();
        void stop();
        bool notify(const Magic::Packets::Packet_T &message);
    }
}
