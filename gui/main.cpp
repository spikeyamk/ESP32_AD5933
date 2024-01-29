#include <thread>
#include <chrono>
#include <iostream>
#include <stop_token>

#include <boost/process.hpp>

#include "gui/imgui_sdl.hpp"
#include "magic/packets.hpp"

#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/test.hpp"

int main(int argc, char* argv[]) {
    const boost::filesystem::path ble_client_path {
    #ifdef _MSC_VER
        "C:/Users/janco/source/repos/ESP32_AD5933/gui/build/ble_client/Debug/ble_client.exe"
    #else
        "/home/spikeyamk/Documents/git-repos/ESP32_AD5933/gui/build/ble_client/ble_client"
    #endif
    };
    boost::interprocess::shared_memory_object::remove(BLE_Client::SHM::Names::shm);
    auto shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
    boost::process::ipstream stdout_stream;
    boost::process::ipstream stderr_stream;
    boost::process::child ble_client { ble_client_path, boost::process::std_out > stdout_stream, boost::process::std_err > stderr_stream };

    bool done = false;
    GUI::run(done, ble_client, shm, stdout_stream, stderr_stream);

    return 0;
}
