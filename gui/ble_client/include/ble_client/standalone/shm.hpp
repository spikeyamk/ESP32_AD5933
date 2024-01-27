#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <cstdlib>
#include <string_view>
#include <string>
#include <optional>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>

#include "ble_client/standalone/events.hpp"
#include "ble_client/standalone/states.hpp"
#include "ble_client/standalone/device.hpp"

#include "ble_client/standalone/state_machines/events_variant.hpp"
#include "ble_client/standalone/state_machines/adapter/states.hpp"
#include "magic/packets.hpp"

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

        template<typename T>
        class Channel {
        protected:
            using Allocator = boost::interprocess::allocator<T, boost::interprocess::managed_shared_memory::segment_manager>;
            using Deque = boost::interprocess::deque<T, Allocator>;
            Deque* deque;
            boost::interprocess::named_mutex mutex;
            boost::interprocess::named_condition condition;
            static constexpr std::string_view deque_prefix     { "BLE_Client.SHM.Channel.deque." };
            static constexpr std::string_view mutex_prefix     { "BLE_Client.SHM.Channel.mutex." };
            static constexpr std::string_view condition_prefix { "BLE_Client.SHM.Channel.condition." };
        public:
            Channel(const std::string_view& name, Deque* deque) :
                deque{ deque },
                mutex{
                    boost::interprocess::open_or_create,
                    std::string(mutex_prefix).append(name).c_str()
                },
                condition{
                    boost::interprocess::open_or_create,
                    std::string(condition_prefix).append(name).c_str()
                }
            {}
        protected:
            void p_send(const T& message) {
                deque->push_back(message);
                condition.notify_one();
            }   

            T p_read() {
                boost::interprocess::scoped_lock lock(mutex);
                condition.wait(lock, [&]() { return !deque->empty(); });
                const T ret = deque->front();
                deque->pop_front();
                return ret;
            }

            std::optional<T> p_read_for(const boost::posix_time::milliseconds& timeout) {
                boost::interprocess::scoped_lock lock(mutex);
                if(condition.timed_wait(lock, boost::get_system_time() + timeout, [&]() { return !deque->empty(); }) == false) {
                    return std::nullopt;
                }
                const T ret = deque->front();
                deque->pop_front();
                return ret;
            }
        };

        template<typename T>
        class InitChannel : public Channel<T> {
        protected:
            using Allocator = boost::interprocess::allocator<T, boost::interprocess::managed_shared_memory::segment_manager>;
            using Deque = boost::interprocess::deque<T, Allocator>;
            Deque* get_init_deque_ptr() {
                return this->deque;
            }
        public:
            InitChannel(const char* name, boost::interprocess::managed_shared_memory& segment) :
                Channel<T>{
                    name,
                    [&]() {
                        const auto tmp_name { std::string(Channel<T>::deque_prefix).append(name) };
                        auto* ret = segment.construct<Deque>(tmp_name.c_str())(segment.get_segment_manager());
                        if(ret == nullptr) {
                            throw std::invalid_argument(std::string("BLE_Client::SHM::InitChannel: could not init to: ").append(tmp_name));
                        }
                        return ret;
                    }()
                }
            {}
        };

        template<typename T>
        class AttachChannel : public Channel<T> {
        protected:
            using Allocator = boost::interprocess::allocator<T, boost::interprocess::managed_shared_memory::segment_manager>;
            using Deque = boost::interprocess::deque<T, Allocator>;
            Deque* get_attach_deque_ptr() {
                return this->deque;
            }
        public:
            AttachChannel(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
                Channel<T>{
                    name,
                    [&]() {
                        auto tmp_name { std::string(Channel<T>::deque_prefix).append(name) };
                        auto* ret = segment.find<Deque>(tmp_name.c_str()).first;
                        if(ret == nullptr) {
                            throw std::invalid_argument(std::string("BLE_Client::SHM::AttachChannel: could not attach to: ").append(tmp_name));
                        }
                        return ret;
                    }()
                }
            {}
        };

        template<typename T>
        class ScopedChannel : public Channel<T> {
        private:
            const std::string deque_name;
            const std::string mutex_name;
            const std::string condition_name;
            boost::interprocess::managed_shared_memory& segment;
        protected:
            using Allocator = boost::interprocess::allocator<T, boost::interprocess::managed_shared_memory::segment_manager>;
            using Deque = boost::interprocess::deque<T, Allocator>;
        public:
            ScopedChannel(const char* name, boost::interprocess::managed_shared_memory& segment) :
                Channel<T>{ name, nullptr },
                deque_name{ std::string(Channel<T>::deque_prefix).append(name) },
                mutex_name{ std::string(Channel<T>::mutex_prefix).append(name) },
                condition_name{ std::string(Channel<T>::condition_prefix).append(name) },
                segment{ segment }
            {}

            ~ScopedChannel() {
                segment.destroy<Deque>(deque_name.c_str());
                boost::interprocess::named_mutex::remove(mutex_name.c_str());
                boost::interprocess::named_condition::remove(condition_name.c_str());
            }
        };

        template<typename T>
        class TX_Channel : public Channel<T> {
            using Allocator = boost::interprocess::allocator<T, boost::interprocess::managed_shared_memory::segment_manager>;
            using Deque = boost::interprocess::deque<T, Allocator>;
        public:
            TX_Channel(const std::string_view& name, Deque* deque) :
                Channel<T>{ name, deque }
            {}

            void send(const T& message) {
                this->p_send(message);
            }
        };

        template<typename T>
        class RX_Channel : public Channel<T> {
            using Allocator = boost::interprocess::allocator<T, boost::interprocess::managed_shared_memory::segment_manager>;
            using Deque = boost::interprocess::deque<T, Allocator>;
        public:
            RX_Channel(const std::string_view& name, Deque* deque) :
                Channel<T>{ name, deque }
            {}

            T read() {
                return read();
            }

            std::optional<T> read_for(const boost::posix_time::milliseconds& timeout) {
                return this->p_read_for(timeout);
            }
        };

        template<typename T>
        class ScopedInitChannel : public InitChannel<T>, public ScopedChannel<T> {
        public:
            ScopedInitChannel(const char* name, boost::interprocess::managed_shared_memory& segment) :
                InitChannel<T>{ name, segment },
                ScopedChannel<T>{ name, segment }
            {}
        };

        template<typename T>
        class ScopedAttachChannel : public AttachChannel<T>, public ScopedChannel<T> {
        public:
            ScopedAttachChannel(const char* name, boost::interprocess::managed_shared_memory& segment) :
                AttachChannel<T>{ name, segment },
                ScopedChannel<T>{ name, segment }
            {}
        };

        class CMD_ChannelTX : public ScopedInitChannel<BLE_Client::StateMachines::T_EventsVariant>, public TX_Channel<BLE_Client::StateMachines::T_EventsVariant> {
        public:
            using RelationBase = ScopedInitChannel<BLE_Client::StateMachines::T_EventsVariant>;
            using DirectionBase = TX_Channel<BLE_Client::StateMachines::T_EventsVariant>;
            CMD_ChannelTX(const char* name, boost::interprocess::managed_shared_memory& segment);
            void send_killer(const BLE_Client::StateMachines::Killer::Events::T_Variant& event);
            void send_adapter(const BLE_Client::StateMachines::Adapter::Events::T_Variant& event);
        };

        class CMD_ChannelRX : public AttachChannel<BLE_Client::StateMachines::T_EventsVariant>, public RX_Channel<BLE_Client::StateMachines::T_EventsVariant> {
            using RelationBase = AttachChannel<BLE_Client::StateMachines::T_EventsVariant>;
            using DirectionBase = RX_Channel<BLE_Client::StateMachines::T_EventsVariant>;
        public:
            CMD_ChannelRX(const char* name, boost::interprocess::managed_shared_memory& segment);
        };

        class NotifyChannelRX : public ScopedAttachChannel<Magic::Packets::Packet_T>, public RX_Channel<Magic::Packets::Packet_T> {
            using RelationBase = ScopedAttachChannel<Magic::Packets::Packet_T>;
            using DirectionBase = RX_Channel<Magic::Packets::Packet_T>;
        public:
            NotifyChannelRX(const char* name, boost::interprocess::managed_shared_memory& segment);
        };

        class NotifyChannelTX: public InitChannel<Magic::Packets::Packet_T>, public TX_Channel<Magic::Packets::Packet_T> {
            using RelationBase = InitChannel<Magic::Packets::Packet_T>;
            using DirectionBase = TX_Channel<Magic::Packets::Packet_T>;
        public:
            NotifyChannelTX(const char* name, boost::interprocess::managed_shared_memory& segment);
        };

        namespace Names {
            constexpr char shm[] { "BLE_Client.shm" };
            constexpr char cmd_postfix[] { "cmd" };
            constexpr char discovery_devices[] { "BLE_Client.shm.discovery_devices" };
            constexpr char adapter_active_state[] = "BLE_Client.SHM.adapter_active_state";
        }

        class ParentSHM {
        private:
            static constexpr size_t size = 2 << 15;
            boost::interprocess::managed_shared_memory segment;
        public:
            CMD_ChannelTX cmd;
            DiscoveryDevices* discovery_devices;
            std::vector<std::shared_ptr<NotifyChannelRX>> notify_channels;
            BLE_Client::StateMachines::Adapter::States::T_Variant* active_state;
            ParentSHM();
            ~ParentSHM();
            void attach_notify_channel(const char* name);
        };

        class ChildSHM {
        private:
            boost::interprocess::managed_shared_memory segment;
        public: 
            CMD_ChannelRX cmd;
            DiscoveryDevices* discovery_devices;
            std::vector<std::shared_ptr<NotifyChannelTX>> notify_channels;
            BLE_Client::StateMachines::Adapter::States::T_Variant* active_state;
            ChildSHM();
            void init_notify_channel(const char* name);
        };

        class SHM {
        private:
            static constexpr size_t shm_size = 2 << 15;
        public:
            static constexpr char name[] = "BLE_Client.SHM";
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

                    };

                    auto shm = std::make_shared<SHM>(shm_size);

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

            static inline std::shared_ptr<SHM> attach() {
                try {
                    static constexpr auto thrower = [](void* ptr, const char* name) {
                        if(ptr == nullptr) {
                            throw std::invalid_argument(name + std::string(" could not attach"));
                        }
                    };

                    auto shm = std::make_shared<SHM>();

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

            inline void free() {
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

