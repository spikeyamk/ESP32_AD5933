#include <iostream>
#include <array>
#include <cassert>
#include <optional>
#include <bitset>

#include "ble_state_machine.hpp"
#include "ble_states.hpp"
#include "magic_packets.hpp"
#include "packet_types.hpp"

int main() {
    Packet<8, 1> test_packet {
        std::array<std::bitset<8>, 1> { std::bitset<8>{0b1001'0000} },
        &MagicPackets::FrequencySweep::Command::check_for_data_valid
    };
}

