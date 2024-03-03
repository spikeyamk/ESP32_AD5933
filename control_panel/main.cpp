#include "gui/run.hpp"
#include "ble_client/shm/common/clean.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ble_client/child_main.hpp"

int main(int argc, char* argv[]) {
    static const boost::filesystem::path self_path {
        #ifdef _WIN32
        // This is needed in order for the path to work on Windows with UTF-16 special characters (Windows uses const wchar_t* for paths) in the path and whitespace otherwise launching the child process will fail
        []() {
            std::basic_string<boost::filesystem::path::value_type> ret;
            ret.resize(MAX_PATH);
            if(GetModuleFileNameW(NULL, ret.data(), MAX_PATH) == 0) {
                std::cout << "ERROR: GUI: GetModuleFileNameW(NULL, ret.data(), MAX_PATH); failed: Could not retrieve self_path\n";
                std::exit(EXIT_FAILURE);
            }
            return ret;
        }().data()
        #elif  __linux__
        []() {
            std::string ret;
            ret.resize(PATH_MAX);
            ssize_t len = readlink("/proc/self/exe", ret.data(), ret.size() - 1);
            if(len == -1) {
                return std::string("");
            }

            ret[len] = '\0';
            return ret;
        }()
        #endif
    };

    static constexpr std::string_view ble_client_magic_key { "okOvDRmWjEUErr3grKvWKpHw2Z0c8L5p6rjl5KT4HAoRGenjFFdPxegc43vCt8BR9ZdWJIPiaMaTYwhr6TMu4od0gHO3r1f7qTQ8pmaQtEm12SqT3IikKLdAsAI46N9E" };
    if(argc > 1 && argv[1] == ble_client_magic_key) {
        return BLE_Client::child_main();
    }

    std::atexit([]() { std::cout << "Control Panel: GUI: Parent process finished\n"; });

    std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
    try {
        shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
    } catch(const boost::interprocess::interprocess_exception& e) {
        std::cout << "ERROR: GUI: Failed to open SHM: exception: " << e.what() << std::endl;
        BLE_Client::SHM::clean(self_path.string());
        try {
            shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
        } catch(const boost::interprocess::interprocess_exception& e) {
            std::cout << "ERROR: GUI: Failed to open SHM even after cleaning: exception: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    boost::process::child ble_client;
    //std::jthread t1(BLE_Client::child_main);
    try {
        ble_client = std::move(boost::process::child { self_path, ble_client_magic_key.data() });
        //ble_client = std::move(boost::process::child { "dummy"});
    } catch(const boost::interprocess::interprocess_exception& e) {
        std::cout << "ERROR: GUI: Failed to open ble_client child process: exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    bool done { false };
    GUI::run(done, ble_client, shm);

    for(size_t i = 0, timeout = 60'000; i < timeout && ble_client.running(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::cout << "ERROR: GUI: Child process had trouble exiting: timeout: " << timeout << " ms i: " << i << " ms" << std::endl;
    }
    if(ble_client.running()) {
        std::cout << "ERROR: GUI: Waiting for child process to exit failed: timeout: killing it right now." << std::endl;
        ble_client.terminate();
    }
    return EXIT_SUCCESS;
}