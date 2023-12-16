#pragma once

#include <mutex>

#include "ble/server/server.hpp"
#include "magic/packets.hpp"

namespace BLE {
	struct Sender {
		std::mutex mutex;
		void send(const Magic::Packets::Packet_T &data) const;
	};
}
