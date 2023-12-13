#pragma once

#include <iostream>
#include <mutex>

namespace NimBLE {
	struct Sender {
		std::mutex mutex;

		template<typename T_Data>
		void send(const T_Data &data) const {
			std::cout << "Sender::send: " << data << std::endl;
		}
	};
}