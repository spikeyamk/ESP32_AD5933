#include <iostream>

#include "magic_packets.hpp"
#include "test.hpp"

using namespace MagicPackets;
void test() {
    bool hasDuplicates = false;

    for (std::size_t i = 0; i < magic_packets.size(); ++i) {
        for (std::size_t j = i + 1; j < magic_packets.size(); ++j) {
            if (magic_packets[i] == magic_packets[j]) {
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
