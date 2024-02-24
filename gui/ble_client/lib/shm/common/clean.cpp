#include <fstream>
#include <stdexcept>
#include <functional>
#include <optional>

#include <trielo/trielo.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>

#include "ble_client/shm/common/names.hpp"
#include "ble_client/shm/parent/specialized.hpp"

#include "ble_client/shm/common/clean.hpp"

namespace ns {
    void to_json(json& j, const Channel& p) {
        j = json{
            {"mutex", p.mutex},
            {"condition", p.condition},
        };
    }

    void from_json(const json& j, Channel& p) {
        j.at("mutex").get_to(p.mutex);
        j.at("condition").get_to(p.condition);
    }

    void to_json(json& j, const Channels& p) {
        j = json{ "channels", p.channels_vector };
    }

    void from_json(const json& j, Channels& p) {
        j.at("channels").get_to(p.channels_vector);
    }

    void to_json(json& j, const SHM& p) {
        j = json{ "shm", { p.channels } };
    }

    void from_json(const json& j, SHM& p) {
        j.at("shm").get_to(p.channels);
    }
}

namespace BLE_Client {
    namespace SHM {
        static const std::optional<ns::SHM> read_json(const std::filesystem::path& shm_json_path) {
            try {
                std::ifstream read_file { shm_json_path }; 
                if(read_file.is_open() == false) {
                    return std::nullopt;
                }
                json j;
                read_file >> j;
                return j;
            } catch(const std::exception& e) {
                std::cout << "ERROR: BLE_Client::SHM::ScopedCleaner::~ScopedCleaner(): exception: " << e.what() << std::endl;
                return std::nullopt;
            }
        }

        void clean(const std::filesystem::path& self_path) {
            /* Top SHM segment */
            #ifdef _MSC_VER
                /* Stupid Windows */
                TRIELO_EQ(boost::interprocess::shared_memory_object::remove(Names::shm), true);
            #else
                Trielo::trielo<boost::interprocess::shared_memory_object::remove>(Trielo::OkErrCode(true), Names::shm);
            #endif

            /* CMD_ChannelRX cmd */
            const std::string cmd_mutex_name { std::string(CMD_ChannelTX::mutex_prefix).append(Names::cmd_postfix) };
            #ifdef _MSC_VER
                /* Stupid Windows */
                TRIELO_EQ(boost::interprocess::named_mutex::remove(cmd_mutex_name.c_str()), true);
            #else
                Trielo::trielo<boost::interprocess::named_mutex::remove>(Trielo::OkErrCode(true), cmd_mutex_name.c_str());
            #endif
            const std::string cmd_condition_name { std::string(CMD_ChannelTX::condition_prefix).append(Names::cmd_postfix) };
            #ifdef _MSC_VER
                /* Stupid Windows */
                TRIELO_EQ(boost::interprocess::named_condition::remove(cmd_condition_name.c_str()), true);
            #else
                Trielo::trielo<boost::interprocess::named_condition::remove>(Trielo::OkErrCode(true), cmd_condition_name.c_str());
            #endif

            /* ConsoleChannelTX console */
            const std::string console_mutex_name { std::string(ConsoleChannelRX::mutex_prefix).append(Names::log_postfix) };
            #ifdef _MSC_VER
                /* Stupid Windows */
                TRIELO_EQ(boost::interprocess::named_mutex::remove(console_mutex_name.c_str()), true);
            #else
                Trielo::trielo<boost::interprocess::named_mutex::remove>(Trielo::OkErrCode(true), console_mutex_name.c_str());
            #endif
            const std::string console_condition_name { std::string(ConsoleChannelRX::condition_prefix).append(Names::log_postfix) };
            #ifdef _MSC_VER
                /* Stupid Windows */
                TRIELO_EQ(boost::interprocess::named_condition::remove(console_condition_name.c_str()), true);
            #else
                Trielo::trielo<boost::interprocess::named_condition::remove>(Trielo::OkErrCode(true), console_condition_name.c_str());
            #endif

            /* Additional NotifyChannelsRX */
            const std::optional<ns::SHM> shm_names {
                read_json(
                    std::filesystem::path(self_path).replace_filename(std::string(Names::shm).append(Names::json_postfix))
                )
            };
            if(shm_names.has_value()) {
                for(const ns::Channel& e: shm_names.value().channels.channels_vector) {
                    #ifdef _MSC_VER
                        /* Stupid Windows */
                        TRIELO_EQ(boost::interprocess::named_mutex::remove(e.mutex.c_str()), true);
                        TRIELO_EQ(boost::interprocess::named_condition::remove(e.condition.c_str()), true);
                    #else
                        Trielo::trielo<boost::interprocess::named_mutex::remove>(Trielo::OkErrCode(true), e.mutex.c_str());
                        Trielo::trielo<boost::interprocess::named_condition::remove>(Trielo::OkErrCode(true), e.condition.c_str());
                    #endif
                }
            }
        }
    }
}
