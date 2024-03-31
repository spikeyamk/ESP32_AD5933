#include <iostream>

#include "gui/run.hpp"
#include "ble_client/shm/common/clean.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ble_client/child_main.hpp"

int main(int argc, char* argv[]) {
    std::atexit([]() { std::cout << "Control Panel: GUI: Parent process finished\n"; });

    std::shared_ptr<BLE_Client::SHM::Parent> shm { std::make_shared<BLE_Client::SHM::Parent>() };

    std::jthread ble_client { BLE_Client::child_main, shm };

    bool done { false };
    GUI::run(done, shm);
    ble_client.join();

    return EXIT_SUCCESS;
}