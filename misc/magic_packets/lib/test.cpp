#include <iostream>
#include <cstdint>

#include "magic_packets.hpp"
#include "test.hpp"

void test() {
    bool hasDuplicates = false;

    for(size_t i = 0; i < MagicPackets::magic_packets.size(); ++i) {
        for(size_t j = i + 1; j < MagicPackets::magic_packets.size(); ++j) {
            if (MagicPackets::magic_packets[i] == MagicPackets::magic_packets[j]) {
				std::printf("ERROR: magic_packets[%zu] is equal to magic_packets[%zu]\n", i, j);
                hasDuplicates = true;
                break;
            }
        }
    }

    if(hasDuplicates) {
        std::cout << "ERROR: There are duplicate elements in the array." << std::endl;
    } else {
        std::cout << "Succes: There are no duplicate elements in the array." << std::endl;
    }
}
