#include <array>
#include <bitset>
#include <cstdint>

#include "include/packet_types.hpp"
#include "include/magic_packets.hpp"

NoDataPacket::NoDataPacket(const MagicPackets::MagicPacket_T &in_raw_bytes) :
    raw_bytes {in_raw_bytes}
{}

NoDataPacket::NoDataPacket() = default;

DumpRegistersPacket::DumpRegistersPacket(const std::array<std::bitset<8>, 19> &data_bytes) :
    Packet { data_bytes, &MagicPackets::Debug::Command::dump_all_registers }
{}

ProgramRegistersPacket::ProgramRegistersPacket(const std::array<std::bitset<8>, 12> &data_bytes) :
    Packet { data_bytes, &MagicPackets::Debug::Command::program_all_registers }
{}

Control_HB_Packet::Control_HB_Packet(const std::bitset<8> command_data_byte) :
    Packet { std::array<std::bitset<8>, 1> { command_data_byte }, &MagicPackets::Debug::Command::control_HB_command }
{}

