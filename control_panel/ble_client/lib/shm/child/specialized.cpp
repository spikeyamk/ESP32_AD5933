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
            /*
            #ifdef _MSC_VER
                this->data->append(message.c_str());
            #else
                this->data->append(message);
            #endif
            this->condition.notify_one();
            */
            std::cout << message;
        }

    }
}
