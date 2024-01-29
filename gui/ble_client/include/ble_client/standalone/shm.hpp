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
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>

#include "ble_client/standalone/device.hpp"

#include "ble_client/standalone/state_machines/events_variant.hpp"
#include "ble_client/standalone/state_machines/adapter/states.hpp"
#include "magic/packets.hpp"

namespace BLE_Client {
    namespace SHM {
        using DiscoveryDevicesAllocator = boost::interprocess::allocator<BLE_Client::Discovery::Device, boost::interprocess::managed_shared_memory::segment_manager>;
        using DiscoveryDevices = boost::interprocess::vector<BLE_Client::Discovery::Device, DiscoveryDevicesAllocator>;

        template<typename T_Container>
        class T_Channel {
        protected:
            T_Container* data;
            boost::interprocess::named_mutex mutex;
            boost::interprocess::named_condition condition;
            static constexpr std::string_view data_prefix      { "BLE_Client.SHM.T_Channel.data." };
            static constexpr std::string_view mutex_prefix     { "BLE_Client.SHM.T_Channel.mutex." };
            static constexpr std::string_view condition_prefix { "BLE_Client.SHM.T_Channel.condition." };
        public:
            T_Channel(const std::string_view& name, T_Container* data) :
                data{ data },
                mutex{
                    boost::interprocess::open_or_create,
                    std::string(mutex_prefix).append(name).c_str()
                },
                condition{
                    boost::interprocess::open_or_create,
                    std::string(condition_prefix).append(name).c_str()
                }
            {}
            T_Channel() = default;
        };

        template<typename T_Container>
        class T_InitChannel : public T_Channel<T_Container> {
        protected:
            T_Container* get_init_ptr() {
                return this->data;
            }
        public:
            T_InitChannel (const char* name, boost::interprocess::managed_shared_memory& segment) :
                T_Channel<T_Container>{
                    name,
                    [&]() {
                        const auto tmp_name { std::string(T_Channel<T_Container>::data_prefix).append(name) };
                        auto* ret = segment.construct<T_Container>(tmp_name.c_str())(segment.get_segment_manager());
                        if(ret == nullptr) {
                            throw std::invalid_argument(std::string("BLE_Client::SHM::InitChannel: could not init to: ").append(tmp_name));
                        }
                        return ret;
                    }()
                }
            {}
        };

        template<typename T_Container>
        class T_AttachChannel : public T_Channel<T_Container> {
            T_Container* get_init_ptr() {
                return this->data;
            }
        public:
            T_AttachChannel(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
                T_Channel<T_Container>{
                    name,
                    [&]() {
                        auto tmp_name { std::string(T_Channel<T_Container>::data_prefix).append(name) };
                        auto* ret = segment.find<T_Container>(tmp_name.c_str()).first;
                        if(ret == nullptr) {
                            throw std::invalid_argument(std::string("BLE_Client::SHM::AttachChannel: could not attach to: ").append(tmp_name));
                        }
                        return ret;
                    }()
                }
            {}
        };

        template<typename T_Container>
        class T_ScopedChannel : public T_Channel<T_Container> {
        public:
            const std::string data_name;
            const std::string mutex_name;
            const std::string condition_name;
            boost::interprocess::managed_shared_memory& segment;
        public:
            T_ScopedChannel(const char* name, boost::interprocess::managed_shared_memory& segment) :
                T_Channel<T_Container>{ name, nullptr },
                data_name{ std::string(T_Channel<T_Container>::data_prefix).append(name) },
                mutex_name{ std::string(T_Channel<T_Container>::mutex_prefix).append(name) },
                condition_name{ std::string(T_Channel<T_Container>::condition_prefix).append(name) },
                segment{ segment }
            {}

            ~T_ScopedChannel() {
                segment.destroy<T_Channel<T_Container>>(data_name.c_str());
                boost::interprocess::named_mutex::remove(mutex_name.c_str());
                boost::interprocess::named_condition::remove(condition_name.c_str());
            }           
        };

        template<typename T_Container>
        class T_ScopedInitChannel : public T_InitChannel<T_Container>, public T_ScopedChannel<T_Container> {
        public:
            T_ScopedInitChannel(const char* name, boost::interprocess::managed_shared_memory& segment) :
                T_InitChannel<T_Container>{ name, segment },
                T_ScopedChannel<T_Container>{ name, segment }
            {}
        };

        template<typename T_Container>
        class T_ScopedAttachChannel : public T_AttachChannel<T_Container>, public T_ScopedChannel<T_Container> {
        public:
            T_ScopedAttachChannel(const char* name, boost::interprocess::managed_shared_memory& segment) :
                T_AttachChannel<T_Container>{ name, segment },
                T_ScopedChannel<T_Container>{ name, segment }
            {}
        };

        template<typename T>
        using Deque = boost::interprocess::deque<T, boost::interprocess::allocator<T, boost::interprocess::managed_shared_memory::segment_manager>>;

        template<typename T>
        using DequeChannel = T_Channel<Deque<T>>;

        template<typename T>
        class TX_DequeChannel : public DequeChannel<T> {
        protected:
        public:
            TX_DequeChannel(const std::string_view& name, DequeChannel<T>* deque) :
                DequeChannel<T> { name, deque }
            {}

            void send(const T& message) {
                this->data->push_back(message);
                this->condition.notify_one();
            }
        };

        template<typename T>
        class RX_DequeChannel : public DequeChannel<T> {
        protected:
        public:
            RX_DequeChannel(const std::string_view& name, DequeChannel<T>* deque) :
                DequeChannel<T> { name, deque }
            {}

            T read() {
                boost::interprocess::scoped_lock lock(this->mutex);
                this->condition.wait(lock, [&]() { return !this->data->empty(); });
                const T ret = this->data->front();
                this->data->pop_front();
                return ret;
            }

            std::optional<T> read_for(const boost::posix_time::milliseconds& timeout) {
                boost::interprocess::scoped_lock lock(this->mutex);
                if(this->condition.timed_wait(lock, boost::get_system_time() + timeout, [&]() { return !this->data->empty(); }) == false) {
                    return std::nullopt;
                }
                const T ret = this->data->front();
                this->data->pop_front();
                return ret;
            }
        };

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

        class LogChannelRX : public ScopedInitChannel<Magic::Packets::Packet_T>, public RX_Channel<Magic::Packets::Packet_T> {
            using RelationBase = ScopedInitChannel<Magic::Packets::Packet_T>;
            using DirectionBase = RX_Channel<Magic::Packets::Packet_T>;
        public:
            LogChannelRX(const char* name, boost::interprocess::managed_shared_memory& segment);
        };

        class LogChannelTX : public AttachChannel<Magic::Packets::Packet_T>, public TX_Channel<Magic::Packets::Packet_T> {
            using RelationBase = AttachChannel<Magic::Packets::Packet_T>;
            using DirectionBase = TX_Channel<Magic::Packets::Packet_T>;
        public:
            LogChannelTX(const char* name, boost::interprocess::managed_shared_memory& segment);
        };

        namespace Names {
            constexpr char shm[] { "BLE_Client.shm" };
            constexpr char cmd_postfix[] { "cmd" };
            constexpr char log_postfix[] { "log" };
            constexpr char discovery_devices[] { "BLE_Client.shm.discovery_devices" };
            constexpr char adapter_active_state[] = "BLE_Client.SHM.adapter_active_state";
        }

        class ParentSHM {
        private:
            static constexpr size_t size = 2 << 15;
            boost::interprocess::managed_shared_memory segment;
        public:
            CMD_ChannelTX cmd;
            LogChannelRX log;
            DiscoveryDevices* discovery_devices;
            std::vector<std::shared_ptr<NotifyChannelRX>> notify_channels;
            BLE_Client::StateMachines::Adapter::States::T_Variant* active_state;
            ParentSHM();
            ~ParentSHM();
            void attach_notify_channel(const BLE_Client::StateMachines::Connector::Events::connect& connect_event);
            void send_packet(const size_t index, const Magic::Packets::Packet_T &packet);

            template<size_t N>
            void send_packet_and_footer(const size_t index, const std::array<uint8_t, N> &raw_array, const Magic::Packets::Packet_T &footer) {
                static_assert(N < Magic::Packets::Debug::start.size());
                Magic::Packets::Packet_T buf = footer;
                std::copy(raw_array.begin(), raw_array.end(), buf.begin());
                send_packet(index, buf);
            }
        };

        class ChildSHM {
        private:
            boost::interprocess::managed_shared_memory segment;
        public: 
            CMD_ChannelRX cmd;
            LogChannelTX log;
            DiscoveryDevices* discovery_devices;
            std::vector<std::shared_ptr<NotifyChannelTX>> notify_channels;
            BLE_Client::StateMachines::Adapter::States::T_Variant* active_state;
            ChildSHM();
            void init_notify_channel(const BLE_Client::StateMachines::Connector::Events::connect& connect_event);
        };
    }
}

