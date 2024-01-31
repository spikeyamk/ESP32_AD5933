#include "ble_client/shm/child/specialized.hpp"

namespace BLE_Client {
    namespace SHM {
        CMD_ChannelRX::CMD_ChannelRX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
            RelationBase{ name, segment },
            DirectionBase{ name, get_attach_ptr() }
        {}

        NotifyChannelTX::NotifyChannelTX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
            RelationBase{ name, segment },
            DirectionBase{ name, get_init_ptr() }
        {}

        ConsoleChannelTX::ConsoleChannelTX(const std::string_view& name, boost::interprocess::managed_shared_memory& segment) :
            Base{ name, segment }
        {}

        void ConsoleChannelTX::log(const std::string& message) {
            this->data->append(message);
            this->condition.notify_one();
            std::cout << message;
        }

    }
}