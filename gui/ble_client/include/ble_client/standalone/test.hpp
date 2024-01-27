#pragma once

#include <thread>
#include <chrono>
#include <iostream>

#include <boost/process.hpp>
#include <boost/thread/thread_time.hpp>
#include <trielo/trielo.hpp>

#include "magic/packets.hpp"
#include "ble_client/standalone/shm.hpp"
#include "ble_client/standalone/events.hpp"

namespace BLE_Client {
    inline void periodic_reader(std::shared_ptr<BLE_Client::SHM::SHM> shm, boost::process::child &client) {
        while(client.running()) {
            std::cout << "periodic_reader: size: " << shm->discovery_devices->size() << std::endl;
            if(shm->discovery_devices->size() > 0) {
                shm->send_cmd(BLE_Client::Discovery::Events::stop_discovery{});
                std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

                /*
                std::for_each(shm->discovery_devices->begin(), shm->discovery_devices->end(), [&shm] (const auto& e) {
                    std::cout << "Found device: identifier: " << e.identifier.data() << ", address: " << e.address.data() << ", connected?: " << e.connected << std::endl;
                });
                */

                //shm->send_cmd(BLE_Client::Discovery::Events::connect{ shm->discovery_devices->begin()->address });
                std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

                shm->send_cmd(BLE_Client::Discovery::Events::is_connected{});
                std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

                shm->send_cmd(BLE_Client::Discovery::Events::is_esp32_ad5933{});
                std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

                shm->send_cmd(BLE_Client::Discovery::Events::esp32_ad5933_write{Magic::Packets::Debug::start});
                shm->send_cmd(BLE_Client::Discovery::Events::esp32_ad5933_write{Magic::Packets::Debug::dump_all_registers});
                shm->send_cmd(BLE_Client::Discovery::Events::esp32_ad5933_write{Magic::Packets::Debug::dump_all_registers});
                shm->send_cmd(BLE_Client::Discovery::Events::esp32_ad5933_write{Magic::Packets::Debug::dump_all_registers});
                shm->send_cmd(BLE_Client::Discovery::Events::esp32_ad5933_write{Magic::Packets::Debug::dump_all_registers});
                shm->send_cmd(BLE_Client::Discovery::Events::esp32_ad5933_write{Magic::Packets::Debug::dump_all_registers});
                shm->send_cmd(BLE_Client::Discovery::Events::esp32_ad5933_write{Magic::Packets::Debug::end});

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

    inline void notify_reporter(std::shared_ptr<BLE_Client::SHM::SHM> shm, boost::process::child& client) {
        using namespace boost::interprocess;
        while(client.running()) {
            scoped_lock<named_mutex> lock(shm->notify_deque_mutex);
            shm->notify_deque_condition.timed_wait(
                lock,
                boost::get_system_time() + boost::posix_time::milliseconds(100),
                [&shm]() { return !shm->notify_deque->empty(); }
            );

            while(shm->notify_deque->empty() == false) {
                std::printf("gui::notify_reporter: caught an incoming notify packet:");
                std::for_each(shm->notify_deque->begin()->begin(), shm->notify_deque->begin()->end(), [index = 0](const uint8_t e) mutable {
                    if(index % 8 == 0) {
                        std::printf("\n\t");
                    }
                    std::printf("0x%02X, ", e);
                    index++;
                });
                std::printf("\n");
                shm->notify_deque->pop_front();
            }
        }
    }

    inline void test(boost::process::child &client, std::shared_ptr<BLE_Client::SHM::SHM> shm) {
        shm->send_cmd(BLE_Client::Discovery::Events::find_default_active_adapter{});
        std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

        shm->send_cmd(BLE_Client::Discovery::Events::start_discovery{});
        std::this_thread::sleep_for(std::chrono::milliseconds(2'000));

        std::jthread reader_thread(periodic_reader, shm, std::ref(client));
        std::jthread notify_thread(notify_reporter, shm, std::ref(client));
    }
}