#include <iostream>
#include <cstdint>
#include <algorithm>

#include "magic/packets.hpp"
#include "magic/tests.hpp"

namespace Magic {
    namespace Packets {
        namespace Tests {
            int packet_duplication() {
                bool hasDuplicates = false;

                for(size_t i = 0; i < Magic::Packets::all_packets.size(); ++i) {
                    for(size_t j = i + 1; j < Magic::Packets::all_packets.size(); ++j) {
                        if(Magic::Packets::all_packets[i].get() == Magic::Packets::all_packets[j].get()) {
                            std::printf("ERROR: all_packets[%zu] is equal to all_packets[%zu]\n", i, j);
                            hasDuplicates = true;
                        }
                    }
                }

                if(hasDuplicates) {
                    std::cout << "ERROR: There are duplicate magic packets in the array." << std::endl;
                    return -1;
                } else {
                    std::cout << "Succes: There are no duplicate magic packets in the array." << std::endl;
                    return 0;
                }

            }

            int run_get_magic_packet_pointer() {
                bool hasErrors = false;

                std::for_each(Magic::Packets::all_packets.begin(), Magic::Packets::all_packets.end(), [index = 0, &hasErrors](const auto &e) mutable {
                    if(Magic::Packets::get_magic_packet_pointer(e) != e.get()) {
                        std::printf("ERROR: get_magic_packet_pointer[%d] return_value is not equal to Magic::Packets::all_packets.[%d]\n", index, index);
                        hasErrors = true;
                    }
                    index++;
                });

                if(hasErrors) {
                    std::cout << "ERROR: run_get_magic_packet_pointer." << std::endl;
                    return -1;
                } else {
                    std::cout << "Succes: run_get_magic_packet_pointer." << std::endl;
                    return 0;
                }
            }

            int check_for_0xFF() {
                bool hasMoreThanOne0xFF = false;

                std::for_each(Magic::Packets::all_packets.begin(), Magic::Packets::all_packets.end(), [index = 0, &hasMoreThanOne0xFF](const auto &e) mutable {
                    if(Magic::Packets::find_footer_start_index(e).has_value()) {
                        const size_t count = std::count(e.get().begin(), e.get().end(), 0xFF);
                        if(count > 1) {
                            std::printf("ERROR: check_for_0xFF[%d] is a data holder packet and has more than one 0xFF footer starter\n", index);
                            hasMoreThanOne0xFF = true;
                        }
                    } else {
                        const size_t count = std::count(e.get().begin(), e.get().end(), 0xFF);
                        if(count != 0) {
                            std::printf("ERROR: check_for_0xFF[%d] is a command holder packet and has nonzero 0xFF footer starters\n", index);
                            hasMoreThanOne0xFF = true;
                        }
                    }
                    index++;
                });

                if(hasMoreThanOne0xFF) {
                    std::cout << "ERROR: check_for_0xFF." << std::endl;
                    return -1;
                } else {
                    std::cout << "Succes: check_for_0xFF." << std::endl;
                    return 0;
                }
            }
        }
    }
}
