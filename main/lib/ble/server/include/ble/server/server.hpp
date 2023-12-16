#pragma once

#include <cstdint>
#include <mutex>

#include "magic/packets.hpp"

namespace BLE {
    namespace Server {
        void run();
        void advertise();
        void stop();
        bool notify(const Magic::Packets::Packet_T &message);

        class Sender {
        public:
            std::mutex mutex;
            Sender() = default;

            inline bool notify(const std::array<uint8_t, Magic::Packets::Debug::start.size()> &message) const {
                return BLE::Server::notify(message);
            }

            template<size_t n_bytes>
            inline bool notify_with_footer(const std::array<uint8_t, n_bytes> &message, const Magic::Packets::Packet_T &footer) const {
                static_assert(n_bytes <= Magic::Packets::Debug::start.size(), "n_bytes must be less than or equal to sizeof(Packet_T)");
                auto send_buf = footer;
                std::copy(message.begin(), message.end(), send_buf.begin());
                return BLE::Server::notify(send_buf);
            }
        };
    }
}
