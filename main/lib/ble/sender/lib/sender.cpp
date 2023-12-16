#include <iostream>

#include "ble/sender/sender.hpp"


namespace BLE {
	void Sender::send(const Magic::Packets::Packet_T &data) const {
		std::cout << "BLE::Sender::send: " << std::endl;
	}
}