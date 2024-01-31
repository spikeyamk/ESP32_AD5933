#include "ble_client/ostream_overloads.hpp"

std::ostream& operator<<(std::ostream& os, SimpleBLE::Peripheral& peripheral) {
    os << "SimpleBLE: Peripheral Information:" << std::endl;
    os << "\tInitialized: " << peripheral.initialized() << std::endl;
    os << "\tIdentifier: " << peripheral.identifier() << std::endl;
    os << "\tAddress: " << peripheral.address() << std::endl;
    os << "\tAddress Type: " << peripheral.address_type() << std::endl;
    os << "\tRSSI: " << peripheral.rssi() << " dBm" << std::endl;
    os << "\tTX Power: " << peripheral.tx_power() << " dBm" << std::endl;
    os << "\tMTU: " << peripheral.mtu() << std::endl;
    os << "\tConnected: " << peripheral.is_connected() << std::endl;
    os << "\tConnectable: " << peripheral.is_connectable() << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, SimpleBLE::Adapter& adapter) {
    os << "SimpleBLE: Adapter Information:" << std::endl;
    os << "\tAdapter Address: " << adapter.address() << std::endl;
    os << "\tAdapter Identifier: " << adapter.identifier() << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, SimpleBLE::Service& service) {
    os << "SimpleBLE: Service Information:" << std::endl;
    os << "\tService UUID: " << service.uuid() << std::endl;
    os << "\tService data: " << service.data() << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, SimpleBLE::Characteristic& characteristic) {
    os << "SimpleBLE: Characteristic Information:" << std::endl;
    os << "\tCharacteristic UUID: " << characteristic.uuid() << std::endl;
    os << "\tCharacteristic can_read: " << characteristic.can_read() << std::endl;
    os << "\tCharacteristic can_write_request: " << characteristic.can_write_request() << std::endl;
    os << "\tCharacteristic can_write_command: " << characteristic.can_write_command() << std::endl;
    os << "\tCharacteristic can_notify: " << characteristic.can_notify() << std::endl;
    os << "\tCharacteristic can_indicate: " << characteristic.can_indicate() << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, SimpleBLE::Descriptor& descriptor) {
    os << "SimpleBLE: Descriptor Information: Descriptor UUID: " << descriptor.uuid() << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, std::vector<SimpleBLE::Adapter> &adapters) {
    for(auto &i: adapters) {
        os << i << std::endl;
    }
    return os;
}
