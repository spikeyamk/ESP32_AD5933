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

#include "ble_client/standalone/state_machines/events_variant.hpp"

namespace BLE_Client {
    namespace SHM {
        typedef boost::interprocess::allocator<BLE_Client::Discovery::Events::T_Variant, boost::interprocess::managed_shared_memory::segment_manager> CMD_DequeAllocator;
        typedef boost::interprocess::deque<BLE_Client::Discovery::Events::T_Variant, CMD_DequeAllocator> CMD_Deque;
        typedef boost::interprocess::allocator<BLE_Client::Discovery::Device, boost::interprocess::managed_shared_memory::segment_manager> DiscoveryDevicesAllocator;
        typedef boost::interprocess::vector<BLE_Client::Discovery::Device, DiscoveryDevicesAllocator> DiscoveryDevices;
        typedef boost::interprocess::allocator<std::array<uint8_t, 20>, boost::interprocess::managed_shared_memory::segment_manager> NotifyAllocator;
        typedef boost::interprocess::deque<std::array<uint8_t, 20>, NotifyAllocator> NotifyDeque;
        typedef boost::interprocess::allocator<BLE_Client::StateMachines::T_EventsVariant, boost::interprocess::managed_shared_memory::segment_manager> UNICMD_DequeAllocator;
        typedef boost::interprocess::deque<BLE_Client::StateMachines::T_EventsVariant, UNICMD_DequeAllocator> UNICMD_Deque;

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
        private:
            static constexpr size_t shm_size = 2 << 15;
        public:
            static constexpr char name[] = "BLE_Client.SHM";
            static constexpr char channel_name[] = "BLE_Client.SHM.channel";
            static constexpr char cmd_deque_name[] = "BLE_Client.SHM.cmd_deque";
            static constexpr char cmd_deque_mutex_name[] = "BLE_Client.SHM.cmd_deque_mutex";
            static constexpr char cmd_deque_condition_name[] = "BLE_Client.SHM.cmd_deque_condition";
            static constexpr char unicmd_deque_name[] = "BLE_Client.SHM.unicmd_deque";
            static constexpr char unicmd_deque_mutex_name[] = "BLE_Client.SHM.unicmd_deque_mutex";
            static constexpr char unicmd_deque_condition_name[] = "BLE_Client.SHM.unicmd_deque_condition";
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
            
            BLE_Client::SHM::UNICMD_Deque* unicmd_deque = nullptr;
            boost::interprocess::named_mutex unicmd_deque_mutex { boost::interprocess::open_or_create, unicmd_deque_mutex_name };
            boost::interprocess::named_condition unicmd_deque_condition { boost::interprocess::open_or_create, unicmd_deque_condition_name };

            BLE_Client::SHM::DiscoveryDevices* discovery_devices = nullptr;

            BLE_Client::Discovery::States::T_State* active_state = nullptr;

            BLE_Client::SHM::NotifyDeque* notify_deque = nullptr;
            boost::interprocess::named_mutex notify_deque_mutex { boost::interprocess::open_or_create, notify_deque_mutex_name };
            boost::interprocess::named_condition notify_deque_condition { boost::interprocess::open_or_create, notify_deque_condition_name };
        public:
            inline SHM() :
                segment{ boost::interprocess::managed_shared_memory(boost::interprocess::open_only, name) }
            {}

            inline SHM(const size_t size) :
                segment{boost::interprocess::create_only, name, size}
            {}

            inline ~SHM() {
                segment.destroy<Channel>(channel_name);

                segment.destroy<CMD_Deque>(cmd_deque_name);
                boost::interprocess::named_mutex::remove(cmd_deque_mutex_name);
                boost::interprocess::named_condition::remove(cmd_deque_condition_name);

                segment.destroy<UNICMD_Deque>(unicmd_deque_name);
                boost::interprocess::named_mutex::remove(unicmd_deque_mutex_name);
                boost::interprocess::named_condition::remove(unicmd_deque_condition_name);

                segment.destroy<NotifyDeque>(notify_deque_name);
                boost::interprocess::named_mutex::remove(notify_deque_mutex_name);
                boost::interprocess::named_condition::remove(notify_deque_condition_name);

                boost::interprocess::shared_memory_object::remove(name);
            }

            inline void send_cmd(BLE_Client::Discovery::Events::T_Variant&& event) {
                cmd_deque->push_back(event);
                cmd_deque_condition.notify_one();
            }

            template<typename T_Event>
            inline void send_unicmd_killer(const T_Event&& event) {
                unicmd_deque->push_back(BLE_Client::StateMachines::Killer::Events::T_Variant{event});
                unicmd_deque_condition.notify_one();
            }

            template<typename T_Event>
            inline void send_unicmd_adapter(const T_Event&& event) {
                unicmd_deque->push_back(BLE_Client::StateMachines::Adapter::Events::T_Variant{event});
                unicmd_deque_condition.notify_one();
            }

            template<typename T_Event>
            inline void send_unicmd_connection(const T_Event&& event) {
                unicmd_deque->push_back(BLE_Client::StateMachines::Connection::Events::T_Variant{event});
                unicmd_deque_condition.notify_one();
            }

            inline void send_notify(std::array<uint8_t, 20> &packet) {
                notify_deque->push_back(packet);
                notify_deque_condition.notify_one();
            }

            static inline std::shared_ptr<SHM> init() {
                try {
                    static const auto thrower = [](void* ptr, const char* name) {
                        if(ptr == nullptr) {
                            throw std::invalid_argument(name + std::string(" was previously used"));
                        }
                    };

                    auto shm = std::make_shared<SHM>(shm_size);

                    shm->channel = shm->segment.construct<Channel>
                        (channel_name)
                        (0, 0.0f);
                    thrower(reinterpret_cast<void*>(shm->channel), channel_name);

                    const CMD_DequeAllocator deque_allocator(shm->segment.get_segment_manager());
                    shm->cmd_deque = shm->segment.construct<CMD_Deque>(cmd_deque_name)(deque_allocator);
                    thrower(reinterpret_cast<void*>(shm->cmd_deque), cmd_deque_name);

                    const UNICMD_DequeAllocator unideque_allocator(shm->segment.get_segment_manager());
                    shm->unicmd_deque = shm->segment.construct<UNICMD_Deque>(unicmd_deque_name)(unideque_allocator);
                    thrower(reinterpret_cast<void*>(shm->unicmd_deque), unicmd_deque_name);

                    const NotifyAllocator notify_allocator(shm->segment.get_segment_manager());
                    shm->notify_deque = shm->segment.construct<NotifyDeque>(notify_deque_name)(notify_allocator);
                    thrower(reinterpret_cast<void*>(shm->notify_deque), notify_deque_name);

                    const DiscoveryDevicesAllocator discovery_devices_allocator(shm->segment.get_segment_manager());
                    shm->discovery_devices = shm->segment.construct<DiscoveryDevices>(discovery_devices_name)(discovery_devices_allocator);
                    thrower(reinterpret_cast<void*>(shm->discovery_devices), discovery_devices_name);

                    shm->active_state = shm->segment.construct<BLE_Client::Discovery::States::T_State>
                        (active_state_name)
                        (BLE_Client::Discovery::States::off{});
                    thrower(reinterpret_cast<void*>(shm->active_state), active_state_name);

                    return shm;
                } catch(const std::exception& e) {
                    std::cerr << "ERROR: BLE_Client::SHM::SHM::init(): exception: " << e.what() << std::endl;
                    std::exit(1);
                    return nullptr;
                }
            }

            static inline SHM* attach() {
                try {
                    static constexpr auto thrower = [](void* ptr, const char* name) {
                        if(ptr == nullptr) {
                            throw std::invalid_argument(name + std::string(" could not attach"));
                        }
                    };

                    SHM* shm = new SHM {};

                    shm->channel = shm->segment.find<Channel>(channel_name).first;
                    thrower(reinterpret_cast<void*>(shm->channel), channel_name);

                    shm->cmd_deque = shm->segment.find<CMD_Deque>(cmd_deque_name).first;
                    thrower(reinterpret_cast<void*>(shm->cmd_deque), cmd_deque_name);

                    shm->unicmd_deque = shm->segment.find<UNICMD_Deque>(unicmd_deque_name).first;
                    thrower(reinterpret_cast<void*>(shm->unicmd_deque), unicmd_deque_name);

                    shm->notify_deque = shm->segment.find<NotifyDeque>(notify_deque_name).first;
                    thrower(reinterpret_cast<void*>(shm->notify_deque), notify_deque_name);

                    shm->discovery_devices = shm->segment.find<DiscoveryDevices>(discovery_devices_name).first;
                    thrower(reinterpret_cast<void*>(shm->discovery_devices), discovery_devices_name);

                    shm->active_state = shm->segment.find<BLE_Client::Discovery::States::T_State>(active_state_name).first;
                    thrower(reinterpret_cast<void*>(shm->active_state), active_state_name);

                    return shm;
                } catch(const std::exception& e) {
                    std::cerr << "ERROR: BLE_Client::SHM::SHM::attach: exception: " << e.what() << std::endl;
                    std::exit(1);
                } 
            }
        };

        class Remover {
        private:
        public:
            inline Remover() {
                remove();
            }

            inline ~Remover() {
                remove();
            }
        private:
            inline void remove() {
                boost::interprocess::shared_memory_object::remove(BLE_Client::SHM::SHM::name);
                boost::interprocess::named_mutex::remove(BLE_Client::SHM::SHM::cmd_deque_mutex_name);
                boost::interprocess::named_condition::remove(BLE_Client::SHM::SHM::cmd_deque_condition_name);
                boost::interprocess::named_mutex::remove(BLE_Client::SHM::SHM::unicmd_deque_mutex_name);
                boost::interprocess::named_condition::remove(BLE_Client::SHM::SHM::unicmd_deque_mutex_name);
                boost::interprocess::named_mutex::remove(BLE_Client::SHM::SHM::notify_deque_mutex_name);
                boost::interprocess::named_condition::remove(BLE_Client::SHM::SHM::notify_deque_condition_name);
            }
        };
    }
}

