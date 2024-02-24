#include "gui/run.hpp"
#include "ble_client/shm/common/clean.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ble_client/child_main.hpp"

int main(int argc, char* argv[]) {
    const std::string_view ble_client_magic_key { "okOvDRmWjEUErr3grKvWKpHw2Z0c8L5p6rjl5KT4HAoRGenjFFdPxegc43vCt8BR9ZdWJIPiaMaTYwhr6TMu4od0gHO3r1f7qTQ8pmaQtEm12SqT3IikKLdAsAI46N9E" };
    if(argc > 1 && argv[1] == ble_client_magic_key) {
        return BLE_Client::child_main();
    }

    std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
    const std::filesystem::path self_path { argv[0] };
    try {
        shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
    } catch(const boost::interprocess::interprocess_exception& e) {
        std::cout << "ERROR: GUI: Failed to open SHM: exception: " << e.what() << std::endl;
        BLE_Client::SHM::clean(self_path);
        try {
            shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
        } catch(const boost::interprocess::interprocess_exception& e) {
            std::cout << "ERROR: GUI: Failed to open SHM even after cleaning: exception: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    boost::process::child ble_client { self_path.string().c_str(), ble_client_magic_key.data() };

    bool done { false };
    GUI::run(done, ble_client, shm);

    for(size_t i = 0; i < 60'000 && ble_client.running(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if(ble_client.running()) {
        ble_client.terminate();
    }
    return EXIT_SUCCESS;
}
