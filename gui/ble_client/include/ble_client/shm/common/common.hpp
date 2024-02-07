#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>

#include "ble_client/device.hpp"

namespace BLE_Client {
    namespace SHM {
        using DiscoveryDevices = boost::interprocess::vector<BLE_Client::Discovery::Device, boost::interprocess::allocator<BLE_Client::Discovery::Device, boost::interprocess::managed_shared_memory::segment_manager>>;
        using String = boost::interprocess::basic_string<char, std::char_traits<char>, boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager>>;
    }
}