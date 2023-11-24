#pragma once

#include <array>
#include <bitset>
#include <optional>
#include <cassert>
#include "magic_packets.hpp"

class NoDataPacket {
private:
    MagicPackets::MagicPacket_T raw_bytes;
public:
    explicit NoDataPacket(const MagicPackets::MagicPacket_T &in_raw_bytes);
    explicit NoDataPacket();
};

template <size_t M, size_t N>
class Packet : public NoDataPacket {
private:
    std::array<std::bitset<M>, N> data;
    const MagicPackets::MagicPacket_T* footer = nullptr;
    std::optional<size_t> footer_start_index = std::nullopt;
public:
    explicit Packet(const std::array<std::bitset<M>, N> &in_data, const MagicPackets::MagicPacket_T *in_footer) :
        NoDataPacket{ assemble_raw_bytes_from_data_n_footer(in_data, in_footer) },
        data {in_data},
        footer {in_footer}
    {}
private:
    MagicPackets::MagicPacket_T assemble_raw_bytes_from_data_n_footer(
        const std::array<std::bitset<M>, N> &in_data,
        const MagicPackets::MagicPacket_T *in_footer
    ) {
        footer_start_index = MagicPackets::find_footer_start_index(*in_footer);
        assert(N == footer_start_index);
        if(footer_start_index.has_value()) {
            MagicPackets::MagicPacket_T result = *in_footer;
            for(size_t i = 0; i < N; i++) {
                result[i] = static_cast<uint8_t>(in_data[i].to_ulong());
            }
            return result;
        } else {
            return *in_footer;
        }
    }
public:
    MagicPackets::MagicPacket_T get_raw_bytes() {
        return raw_bytes;
    }

    const MagicPackets::MagicPacket_T* get_footer_pointer() {
        return footer;
    }
};

class DumpRegistersPacket : public Packet<8, 19> {
    explicit DumpRegistersPacket(const std::array<std::bitset<8>, 19> &data_bytes);
};

class ProgramRegistersPacket : public Packet<8, 12> {
    explicit ProgramRegistersPacket(const std::array<std::bitset<8>, 12> &data_bytes);
};

class Control_HB_Packet  : public Packet<8, 1> {
    explicit Control_HB_Packet(const std::bitset<8> command_data_byte);
};

