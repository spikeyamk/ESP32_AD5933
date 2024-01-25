#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <cstdlib>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "ble_client/standalone/events.hpp"
#include "ble_client/standalone/states.hpp"
#include "ble_client/standalone/device.hpp"

namespace BLE_Client {
    namespace SHM {
        typedef boost::interprocess::allocator<BLE_Client::Discovery::Events::T_Variant, boost::interprocess::managed_shared_memory::segment_manager> CMD_DequeAllocator;
        typedef boost::interprocess::deque<BLE_Client::Discovery::Events::T_Variant, CMD_DequeAllocator> CMD_Deque;
        typedef boost::interprocess::allocator<BLE_Client::Discovery::Device, boost::interprocess::managed_shared_memory::segment_manager> DiscoveryDevicesAllocator;
        typedef boost::interprocess::vector<BLE_Client::Discovery::Device, DiscoveryDevicesAllocator> DiscoveryDevices;
        typedef boost::interprocess::allocator<std::array<uint8_t, 20>, boost::interprocess::managed_shared_memory::segment_manager> NotifyAllocator;
        typedef boost::interprocess::deque<std::array<uint8_t, 20>, NotifyAllocator> NotifyDeque;

        class Channel {
        public:
            int x;
            float y;

            inline Channel(const int x, const float y) :
                x{x},
                y{y}
            {}
        };

        class SHM {
        public:
            static constexpr char name[] = "BLE_Client.SHM";
            static constexpr char channel_name[] = "BLE_Client.SHM::channel";
            static constexpr char cmd_deque_name[] = "BLE_Client.SHM.cmd_deque";
            static constexpr char cmd_deque_mutex_name[] = "BLE_Client.SHM.cmd_deque_mutex";
            static constexpr char cmd_deque_condition_name[] = "BLE_Client.SHM.cmd_deque_condition";
            static constexpr char discovery_devices_name[] = "BLE_Client.SHM.discovery_devices";
            static constexpr char active_state_name[] = "BLE_Client.SHM.active_state_name";
            static constexpr char notify_deque_name[] = "BLE_Client.SHM.notify_deque_name";
            static constexpr char notify_deque_mutex_name[] = "BLE_Client.SHM.notify_deque_mutex_name";
            static constexpr char notify_deque_condition_name[] = "BLE_Client.SHM.notify_deque_condition_name";

            boost::interprocess::managed_shared_memory segment;
            BLE_Client::SHM::Channel* channel = nullptr;

            BLE_Client::SHM::CMD_Deque* cmd_deque = nullptr;
            boost::interprocess::named_mutex cmd_deque_mutex { boost::interprocess::open_or_create, cmd_deque_mutex_name };
            boost::interprocess::named_condition cmd_deque_condition { boost::interprocess::open_or_create, cmd_deque_condition_name };

            BLE_Client::SHM::DiscoveryDevices* discovery_devices = nullptr;

            BLE_Client::Discovery::States::T_State* active_state = nullptr;

            BLE_Client::SHM::NotifyDeque* notify_deque = nullptr;
            boost::interprocess::named_mutex notify_deque_mutex { boost::interprocess::open_or_create, notify_deque_mutex_name };
            boost::interprocess::named_condition notify_deque_condition { boost::interprocess::open_or_create, notify_deque_condition_name };

            inline SHM(const size_t size) :
                segment{boost::interprocess::create_only, name, size}
            {}

            inline SHM() :
                segment{ boost::interprocess::managed_shared_memory(boost::interprocess::open_only, name) }
            {}

            inline void send_cmd(BLE_Client::Discovery::Events::T_Variant&& event) {
                cmd_deque->push_back(event);
                cmd_deque_condition.notify_one();
            }

            inline void send_notify(std::array<uint8_t, 20> &packet) {
                notify_deque->push_back(packet);
                notify_deque_condition.notify_one();
            }

            inline void init() {
                try {
                    channel = segment.construct<BLE_Client::SHM::Channel>
                        (BLE_Client::SHM::SHM::channel_name)
                        (0, 0.0f);
                    const BLE_Client::SHM::CMD_DequeAllocator deque_allocator(segment.get_segment_manager());
                    cmd_deque = segment.construct<BLE_Client::SHM::CMD_Deque>(BLE_Client::SHM::SHM::cmd_deque_name)(deque_allocator);

                    const BLE_Client::SHM::NotifyAllocator notify_allocator(segment.get_segment_manager());
                    notify_deque = segment.construct<BLE_Client::SHM::NotifyDeque>(BLE_Client::SHM::SHM::notify_deque_name)(notify_allocator);

                    const BLE_Client::SHM::DiscoveryDevicesAllocator discovery_devices_allocator(segment.get_segment_manager());
                    discovery_devices = segment.construct<BLE_Client::SHM::DiscoveryDevices>(BLE_Client::SHM::SHM::discovery_devices_name)(discovery_devices_allocator);

                    active_state = segment.construct<BLE_Client::Discovery::States::T_State>(BLE_Client::SHM::SHM::active_state_name)(BLE_Client::Discovery::States::off{});
                } catch(const std::exception& e) {
                    std::cerr << "ERROR: BLE_Client::init_shm(): exception: " << e.what() << std::endl;
                    std::exit(1);
                }
            }

            inline void attach() {
                try {
                    do {
                        channel = segment.find<BLE_Client::SHM::Channel>(BLE_Client::SHM::SHM::channel_name).first;
                    } while(channel == nullptr);

                    do {
                        cmd_deque = segment.find<BLE_Client::SHM::CMD_Deque>(BLE_Client::SHM::SHM::cmd_deque_name).first;
                    } while(cmd_deque == nullptr);

                    do {
                        notify_deque = segment.find<BLE_Client::SHM::NotifyDeque>(BLE_Client::SHM::SHM::notify_deque_name).first;
                    } while(notify_deque == nullptr);

                    do {
                        discovery_devices = segment.find<BLE_Client::SHM::DiscoveryDevices>(BLE_Client::SHM::SHM::discovery_devices_name).first;
                    } while(discovery_devices == nullptr);

                    do {
                        active_state = segment.find<BLE_Client::Discovery::States::T_State>(BLE_Client::SHM::SHM::active_state_name).first;
                    } while(active_state == nullptr);
                } catch(const std::exception& e) {
                    std::cerr << "ERROR: BLE_Client::SHM::attach_shm: exception: " << e.what() << std::endl;
                    std::exit(1);
                } 
            }
        };

        inline std::shared_ptr<BLE_Client::SHM::SHM> attach_shm() {
        }

        class Remover {
        private:
            const char* shm_name = BLE_Client::SHM::SHM::name;
            const char* cmd_deque_mutex_name = BLE_Client::SHM::SHM::cmd_deque_mutex_name;
            const char* cmd_deque_condition_name = BLE_Client::SHM::SHM::cmd_deque_condition_name;
            const char* notify_deque_mutex_name = BLE_Client::SHM::SHM::notify_deque_mutex_name;
            const char* notify_deque_condition_name = BLE_Client::SHM::SHM::notify_deque_condition_name;
        public:
            inline Remover() {
                remove();
            }

            inline ~Remover() {
                remove();
            }
        private:
            inline void remove() {
                boost::interprocess::shared_memory_object::remove(shm_name);
                boost::interprocess::named_mutex::remove(cmd_deque_mutex_name);
                boost::interprocess::named_condition::remove(cmd_deque_condition_name);
                boost::interprocess::named_mutex::remove(notify_deque_mutex_name);
                boost::interprocess::named_condition::remove(notify_deque_condition_name);
            }
        };
    }
}

