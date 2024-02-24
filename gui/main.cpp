#include <boost/filesystem/path.hpp>

#include <utf/utf.hpp>

#include "gui/run.hpp"
#include "ble_client/shm/common/clean.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ble_client/child_main.hpp"

int main(int argc, char* argv[]) {
    // This ugly hack is needed in order for the path to work on Windows with UTF-16 special characters in the path and whitespace otherwise launching the child process will fail
    static const boost::filesystem::path self_path { boost::filesystem::initial_path().append(boost::filesystem::path(argv[0]).filename()) };
    const std::string_view ble_client_magic_key { "okOvDRmWjEUErr3grKvWKpHw2Z0c8L5p6rjl5KT4HAoRGenjFFdPxegc43vCt8BR9ZdWJIPiaMaTYwhr6TMu4od0gHO3r1f7qTQ8pmaQtEm12SqT3IikKLdAsAI46N9E" };
    if(argc > 1 && argv[1] == ble_client_magic_key) {
        return BLE_Client::child_main();
    }

    std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
    try {
        shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
    } catch(const boost::interprocess::interprocess_exception& e) {
        std::cout << "ERROR: GUI: Failed to open SHM: exception: " << e.what() << std::endl;
        BLE_Client::SHM::clean(std::filesystem::path(self_path.string()));
        try {
            shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
        } catch(const boost::interprocess::interprocess_exception& e) {
            std::cout << "ERROR: GUI: Failed to open SHM even after cleaning: exception: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    boost::process::child ble_client { self_path, ble_client_magic_key.data() };

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
