#include <thread>
#include <chrono>
#include <iostream>
#include <string_view>

#include <boost/process.hpp>

#include "gui/run.hpp"

#include "ble_client/shm/common/cleaner.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ble_client/child_main.hpp"

int main(int argc, char* argv[]) {
    static constexpr std::string_view magic_key { "okOvDRmWjEUErr3grKvWKpHw2Z0c8L5p6rjl5KT4HAoRGenjFFdPxegc43vCt8BR9ZdWJIPiaMaTYwhr6TMu4od0gHO3r1f7qTQ8pmaQtEm12SqT3IikKLdAsAI46N9E" };
    if(argc > 1 && argv[1] == magic_key) {
        return BLE_Client::child_main();
    }

    std::shared_ptr<BLE_Client::SHM::ParentSHM> shm = nullptr;
    try {
        shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
    } catch(const boost::interprocess::interprocess_exception& e) {
        std::cout << "ERROR: GUI: Failed to open SHM: exception: " << e.what() << std::endl;
        {
            BLE_Client::SHM::Cleaner cleaner {};
        }
        try {
            shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
        } catch(const boost::interprocess::interprocess_exception& e) {
            std::cout << "ERROR: GUI: Failed to open SHM even after cleaning: exception: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    boost::process::child ble_client { argv[0], magic_key.data() };

    bool done = false;
    GUI::run(done, ble_client, shm);

    for(size_t i = 0; i < 60'000 && ble_client.running(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if(ble_client.running()) {
        ble_client.terminate();
    }
    return 0;
}
