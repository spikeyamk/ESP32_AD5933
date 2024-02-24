#pragma once

#include <cstddef>
#include <string_view>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>

#include "ble_client/shm/common/clean.hpp"
#include "ble_client/shm/common/names.hpp"

namespace BLE_Client {
    namespace SHM {
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
                        const auto tmp_name { std::string(T_Channel<T_Container>::data_prefix).append(name) };
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
                            loaded_shm_names.channels.channels_vector.begin(),
                            loaded_shm_names.channels.channels_vector.end(),
                            [&](const auto& e) {
                                return (e.mutex == mutex_name) && (e.condition) == condition_name;
                            }
                        ) == loaded_shm_names.channels.channels_vector.end()) {
                            loaded_shm_names.channels.channels_vector.push_back(channel);
                            std::ofstream output_file { std::string(Names::shm).append(Names::json_postfix) };
                            const json new_shm_json_names { loaded_shm_names };
                            output_file << std::setw(4) << new_shm_json_names;
                        }
                    } else {
                        const ns::SHM shm_names { ns::Channels { std::vector<ns::Channel> { channel } } };
                        const json shm_names_json { shm_names };
                        std::ofstream output_file { std::string(Names::shm).append(Names::json_postfix) };
                        output_file << std::setw(4) << shm_names_json;
                    }
                } catch(const std::exception& e) {
                    std::cout << "ERROR: BLE_Client::SHM::T_ScopedChannel::add_names_to_json: existing SHM json file corrupted: exception: " << e.what() << std::endl;
                    try {
                        const ns::SHM shm_names { std::vector<ns::Channel> { channel } };
                        const json shm_names_json { shm_names };
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
    }
}