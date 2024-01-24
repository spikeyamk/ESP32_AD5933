#include <thread>
#include <chrono>
#include <iostream>
#include <memory>

#include <boost/process.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#include "ble_client/standalone/shm.hpp"

#include "ble_client/ble_client.hpp"
#include "gui/imgui_sdl.hpp"
#include "magic/packets.hpp"
#include <trielo/trielo.hpp>

void gui() {
    bool done = false;
    //std::jthread gui_thread(GUI::run, std::ref(done));
    GUI::run(done);
}

std::shared_ptr<BLE_Client::SHM::SHM> attach_shm() {
    std::shared_ptr<BLE_Client::SHM::SHM> ret;
    while(1) {
        try {
            ret = std::make_shared<BLE_Client::SHM::SHM>();
            std::printf("no exception\n");
            break;
        } catch(...) {
            std::printf("some exception\n");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    do {
        ret->channel = ret->segment.find<BLE_Client::SHM::Channel>(BLE_Client::SHM::SHM::channel_name).first;
    } while(ret->channel == nullptr);

    do {
        ret->cmd_deque = ret->segment.find<BLE_Client::SHM::CMD_Deque>(BLE_Client::SHM::SHM::cmd_deque_name).first;
    } while(ret->cmd_deque == nullptr);

    do {
        ret->notify_deque = ret->segment.find<BLE_Client::SHM::NotifyDeque>(BLE_Client::SHM::SHM::notify_deque_name).first;
    } while(ret->notify_deque == nullptr);

    do {
        ret->discovery_devices = ret->segment.find<BLE_Client::SHM::DiscoveryDevices>(BLE_Client::SHM::SHM::discovery_devices_name).first;
    } while(ret->discovery_devices == nullptr);

    do {
        ret->active_state = ret->segment.find<BLE_Client::Discovery::States::T_State>(BLE_Client::SHM::SHM::active_state_name).first;
    } while(ret->active_state == nullptr);

    return ret;
}

void periodic_reader(std::shared_ptr<BLE_Client::SHM::SHM> shm, boost::process::child &client) {
    while(client.running()) {
        std::cout << "periodic_reader: size: " << shm->discovery_devices->size() << std::endl;
        if(shm->discovery_devices->size() > 0) {
            shm->send_cmd(BLE_Client::Discovery::Events::stop_discovery{});
            std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

            std::for_each(shm->discovery_devices->begin(), shm->discovery_devices->end(), [&shm] (const auto& e) {
                std::cout << "Found device: identifier: " << e.identifier << ", address: " << e.address << ", connected?: " << e.connected << std::endl;
            });

            shm->send_cmd(BLE_Client::Discovery::Events::connect{ 0 });
            std::this_thread::sleep_for(std::chrono::milliseconds(10'000));

            shm->send_cmd(BLE_Client::Discovery::Events::is_connected{});
            std::this_thread::sleep_for(std::chrono::milliseconds(10'000));

            shm->send_cmd(BLE_Client::Discovery::Events::is_esp32_ad5933{});
            std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

            shm->send_cmd(BLE_Client::Discovery::Events::esp32_ad5933_write{Magic::Packets::Debug::start});
            std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

            shm->send_cmd(BLE_Client::Discovery::Events::stop_using_esp32_ad5933{});
            std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

            shm->send_cmd(BLE_Client::Discovery::Events::disconnect{});
            std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

            shm->send_cmd(BLE_Client::Discovery::Events::kill{});
            std::this_thread::sleep_for(std::chrono::milliseconds(2'000));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void state_reporter(std::shared_ptr<BLE_Client::SHM::SHM> shm, boost::process::child& client) {
    while(client.running()) {
        if(shm->active_state != nullptr) {
            std::visit([](auto& e){
                std::cout << "gui: state_reporter: shm->active_state: " << typeid(e).name() << std::endl;
            }, *shm->active_state);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char* argv[]) {
    #ifdef _MSC_VER
        const boost::filesystem::path client_path { "C:/Users/janco/source/repos/ESP32_AD5933/gui/build/ble_client/Debug/ble_client.exe" };
    #else
        const boost::filesystem::path client_path { "/home/spikeyamk/Documents/git-repos/ESP32_AD5933/gui/build/ble_client/ble_client" };
    #endif
    boost::process::child client { client_path };
    std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

    std::shared_ptr<BLE_Client::SHM::SHM> shm { attach_shm() };
    std::jthread reporter_thread(state_reporter, shm, std::ref(client));

    shm->send_cmd(BLE_Client::Discovery::Events::find_default_active_adapter{});
    std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

    shm->send_cmd(BLE_Client::Discovery::Events::start_discovery{});
    std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

    std::jthread reader_thread(periodic_reader, shm, std::ref(client));

    client.wait();
    return 0;
}
