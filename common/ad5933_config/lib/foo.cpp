#include <iostream>
#include <iomanip>
#include <vector>

#include "foo.hpp"

#include <boost/multiprecision/cpp_int.hpp>

void test_no_backend() {
	using namespace boost::multiprecision;
	using uint9_t = number<cpp_int_backend<9, 9, unsigned_magnitude, unchecked, void> >;
	uint9_t test_number { 0xFFFF'FFFF'FFFF'FFFFu };
	while(1) {
		std::cout << test_number++ << std::endl;
	}
}

void print_hello_world() {
	std::cout << "Hello World!\n";
	test_no_backend();
}
