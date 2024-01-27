#include <iostream>

#include "ble_client/standalone/worker.hpp"

int main(void) {
    std::printf("BLE_Client: process started\n");
	std::atexit([]() { std::printf("BLE_Client: process finished\n"); });
	BLE_Client::worker();
	return 0;
}
