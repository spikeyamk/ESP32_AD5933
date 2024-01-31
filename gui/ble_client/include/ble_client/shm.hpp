#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
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
#include <trielo/trielo.hpp>

#include "ble_client/device.hpp"

#include "ble_client/state_machines/events_variant.hpp"
#include "ble_client/state_machines/adapter/states.hpp"
#include "magic/events/common.hpp"
#include "magic/events/results.hpp"
#include "json/shm.hpp"

namespace BLE_Client {
    namespace SHM {
        using DiscoveryDevicesAllocator = boost::interprocess::allocator<BLE_Client::Discovery::Device, boost::interprocess::managed_shared_memory::segment_manager>;
        using DiscoveryDevices = boost::interprocess::vector<BLE_Client::Discovery::Device, DiscoveryDevicesAllocator>;

        namespace Names {
            constexpr char shm[] { "BLE_Client.shm" };
            constexpr char cmd_postfix[] { "cmd" };
            constexpr char log_postfix[] { "log" };
            constexpr char discovery_devices[] { "BLE_Client.shm.discovery_devices" };
            constexpr char adapter_active_state[] = "BLE_Client.SHM.adapter_active_state";
            constexpr char json_postfix[] { ".json" };
        }

        template<typename T_Container>
        class T_Channel {
        public:
            static constexpr std::string_view data_prefix      { "BLE_Client.SHM.T_Channel.data." };
            static constexpr std::string_view mutex_prefix     { "BLE_Client.SHM.T_Channel.mutex." };
            static constexpr std::string_view condition_prefix { "BLE_Client.SHM.T_Channel.condition." };
        protected:
            T_Container* data;
            boost::interprocess::named_mutex mutex;
            boost::interprocess::named_condition condition;
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

            T_InitChannel<T_Container>* get_init_self() {
                return this;
            }
        public:
            T_InitChannel (const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
                T_Channel<T_Container>{
                    name,
                    [&]() {
                        const auto tmp_name { std::string(T_Channel<T_Container>::data_prefix).append(name) };
                        T_Container* ret = segment.construct<T_Container>(tmp_name.c_str())(segment.get_segment_manager());
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
        protected:
            T_Container* get_attach_ptr() {
                return this->data;
            }
        public:
            T_AttachChannel(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
                T_Channel<T_Container>{
                    name,
                    [&]() {
                        auto tmp_name { std::string(T_Channel<T_Container>::data_prefix).append(name) };
                        T_Container* ret = segment.find<T_Container>(tmp_name.c_str()).first;
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
            T_ScopedChannel(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
                T_Channel<T_Container>{ name, nullptr },
                data_name{ std::string(T_Channel<T_Container>::data_prefix).append(name) },
                mutex_name{ std::string(T_Channel<T_Container>::mutex_prefix).append(name) },
                condition_name{ std::string(T_Channel<T_Container>::condition_prefix).append(name) },
                segment{ segment }
            {
                add_names_to_json();
            }

            ~T_ScopedChannel() {
                segment.destroy<T_Container>(data_name.c_str());
                boost::interprocess::named_mutex::remove(mutex_name.c_str());
                boost::interprocess::named_condition::remove(condition_name.c_str());
            }
        private:
            void add_names_to_json() const {
                const ns::Channel channel { mutex_name, condition_name };
                try {
                    std::ifstream input_file { std::string(Names::shm).append(Names::json_postfix) };
                    if(input_file.is_open()) {
                        json loaded_shm_json_names;
                        input_file >> loaded_shm_json_names;
                        input_file.close();
                        ns::SHM loaded_shm_names = loaded_shm_json_names;
                        if(std::find_if(
                            loaded_shm_names.channels.begin(),
                            loaded_shm_names.channels.end(),
                            [&](const auto& e) {
                                return (e.mutex == mutex_name) && (e.condition) == condition_name;
                            }
                        ) == loaded_shm_names.channels.end()) {
                            loaded_shm_names.channels.push_back(channel);
                            std::ofstream output_file { std::string(Names::shm).append(Names::json_postfix) };
                            const json new_shm_json_names = loaded_shm_names;
                            output_file << std::setw(4) << new_shm_json_names;
                        }
                    } else {
                        const json shm_names_json = ns::SHM{ std::vector<ns::Channel> { channel } };
                        std::ofstream output_file { std::string(Names::shm).append(Names::json_postfix) };
                        output_file << std::setw(4) << shm_names_json;
                    }
                } catch(const std::exception& e) {
                    std::cout << "ERROR: BLE_Client::SHM::T_ScopedChannel::add_names_to_json: existing SHM json file corrupted: exception: " << e.what() << std::endl;
                    try {
                        const json shm_names_json = ns::SHM{ std::vector<ns::Channel> { channel } };
                        std::ofstream output_file { std::string(Names::shm).append(Names::json_postfix) };
                        output_file << std::setw(4) << shm_names_json;
                    } catch(...) {
                        std::cout << "ERROR: BLE_Client::SHM::T_ScopedChannel::add_names_to_json: complete disaster exception: " << std::endl;
                    }
                }
            }
        };

        template<typename T_Container>
        class T_ScopedInitChannel : public T_InitChannel<T_Container>, public T_ScopedChannel<T_Container> {
        public:
            T_ScopedInitChannel(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
                T_InitChannel<T_Container>{ name, segment },
                T_ScopedChannel<T_Container>{ name, segment }
            {}
        };

        template<typename T_Container>
        class T_ScopedAttachChannel : public T_AttachChannel<T_Container>, public T_ScopedChannel<T_Container> {
        public:
            T_ScopedAttachChannel(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
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
            TX_DequeChannel(const std::string_view& name, Deque<T>* deque) :
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
            RX_DequeChannel(const std::string_view& name, Deque<T>* deque) :
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

        class CMD_ChannelTX : public T_ScopedInitChannel<Deque<BLE_Client::StateMachines::T_EventsVariant>>, public TX_DequeChannel<BLE_Client::StateMachines::T_EventsVariant> {
        public:
            using RelationBase = T_ScopedInitChannel<Deque<BLE_Client::StateMachines::T_EventsVariant>>;
            using DirectionBase = TX_DequeChannel<BLE_Client::StateMachines::T_EventsVariant>;
            CMD_ChannelTX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
            void send_killer(const BLE_Client::StateMachines::Killer::Events::T_Variant& event);
            void send_adapter(const BLE_Client::StateMachines::Adapter::Events::T_Variant& event);
        };

        class CMD_ChannelRX : public T_AttachChannel<Deque<BLE_Client::StateMachines::T_EventsVariant>>, public RX_DequeChannel<BLE_Client::StateMachines::T_EventsVariant> {
            using RelationBase = T_AttachChannel<Deque<BLE_Client::StateMachines::T_EventsVariant>>;
            using DirectionBase = RX_DequeChannel<BLE_Client::StateMachines::T_EventsVariant>;
        public:
            CMD_ChannelRX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
        };

        class NotifyChannelRX : public T_ScopedAttachChannel<Deque<Magic::Events::Results::Variant>>, public RX_DequeChannel<Magic::Events::Results::Variant> {
            using RelationBase = T_ScopedAttachChannel<Deque<Magic::Events::Results::Variant>>;
            using DirectionBase = RX_DequeChannel<Magic::Events::Results::Variant>;
        public:
            NotifyChannelRX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
        };

        class NotifyChannelTX: public T_InitChannel<Deque<Magic::Events::Results::Variant>>, public TX_DequeChannel<Magic::Events::Results::Variant> {
            using RelationBase = T_InitChannel<Deque<Magic::Events::Results::Variant>>;
            using DirectionBase = TX_DequeChannel<Magic::Events::Results::Variant>;
        public:
            NotifyChannelTX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
        };

        using String = boost::interprocess::basic_string<char, std::char_traits<char>, boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager>>;

        class ConsoleChannelRX_Interface : public T_InitChannel<String> {
            using Base = T_InitChannel<String>;
        public:
            ConsoleChannelRX_Interface(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
            std::optional<std::string> read_for(const boost::posix_time::milliseconds& timeout_ms);
        };

        class ConsoleChannelRX : public ConsoleChannelRX_Interface, public T_ScopedChannel<String> {
            using DirectionBase = ConsoleChannelRX_Interface;
            using RelationBase = T_ScopedChannel<String>;
        public:
            ConsoleChannelRX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
        };

        class ConsoleChannelTX : public T_AttachChannel<String> {
        private:
            using Base = T_AttachChannel<String>;
        public:
            ConsoleChannelTX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment);
            void log(const std::string& message);
        };

        class ParentSHM {
        private:
            static constexpr size_t size = 2 << 20;
            boost::interprocess::managed_shared_memory segment;
        public:
            CMD_ChannelTX cmd;
            ConsoleChannelRX console;
            DiscoveryDevices* discovery_devices;
            std::vector<std::shared_ptr<NotifyChannelRX>> notify_channels;
            BLE_Client::StateMachines::Adapter::States::T_Variant* active_state;
            ParentSHM();
            ~ParentSHM();
            void attach_notify_channel(const BLE_Client::StateMachines::Connector::Events::connect& connect_event);
        };

        class ChildSHM {
        private:
            boost::interprocess::managed_shared_memory segment;
        public: 
            CMD_ChannelRX cmd;
            ConsoleChannelTX console;
            DiscoveryDevices* discovery_devices;
            std::vector<std::shared_ptr<NotifyChannelTX>> notify_channels;
            BLE_Client::StateMachines::Adapter::States::T_Variant* active_state;
            ChildSHM();
            void init_notify_channel(const BLE_Client::StateMachines::Connector::Events::connect& connect_event);
        };

        class Cleaner {
        private:
            inline std::optional<ns::SHM> read_json() {
                std::ifstream read_file { std::string(Names::shm).append(Names::json_postfix) };
                if(read_file.is_open() == false) {
                    return std::nullopt;
                }

                try {
                    json j;
                    read_file >> j;
                    return j;
                } catch(...) {
                    return std::nullopt;
                }
            }
        public:
            inline ~Cleaner() {
                /* Top SHM segment */
                Trielo::trielo<boost::interprocess::shared_memory_object::remove>(Trielo::OkErrCode(true), Names::shm);

                /* CMD_ChannelRX cmd */
                const auto cmd_mutex_name { std::string(CMD_ChannelRX::mutex_prefix).append(Names::cmd_postfix) };
                Trielo::trielo<boost::interprocess::named_mutex::remove>(Trielo::OkErrCode(true), cmd_mutex_name.c_str());
                const auto cmd_condition_name { std::string(CMD_ChannelRX::condition_prefix).append(Names::cmd_postfix) };
                Trielo::trielo<boost::interprocess::named_condition::remove>(Trielo::OkErrCode(true), cmd_condition_name.c_str());

                /* ConsoleChannelTX console */
                const auto console_mutex_name { std::string(ConsoleChannelTX::mutex_prefix).append(Names::log_postfix) };
                Trielo::trielo<boost::interprocess::named_mutex::remove>(Trielo::OkErrCode(true), console_mutex_name.c_str());
                const auto console_condition_name { std::string(ConsoleChannelTX::condition_prefix).append(Names::log_postfix) };
                Trielo::trielo<boost::interprocess::named_condition::remove>(Trielo::OkErrCode(true), console_condition_name.c_str());

                /* Additional NotifyChannelsRX */
                const std::optional<ns::SHM> shm_names { read_json() };
                if(shm_names.has_value()) {
                    for(const auto& e: shm_names.value().channels) {
                        Trielo::trielo<boost::interprocess::named_mutex::remove>(Trielo::OkErrCode(true), e.mutex.c_str());
                        Trielo::trielo<boost::interprocess::named_condition::remove>(Trielo::OkErrCode(true), e.condition.c_str());
                    }
                }
            }
        };
    }
}

