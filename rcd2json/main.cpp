#include <iostream>
#include <filesystem>
#include <fstream>
#include <array>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "magic/events/results.hpp"

namespace ns {
	struct Record {
		Magic::Events::Results::Auto::Timeval timeval;
		Magic::Events::Results::Auto::Point point;
	};

    void to_json(json& j, const Record& p) {
        j = json {
            { "tv_sec", p.timeval.tv.tv_sec },
            { "tv_usec", p.timeval.tv.tv_usec },

            { "status", p.point.status },
            { "config", p.point.config },
            { "impedance", p.point.impedance },
            { "phase", p.point.phase },
        };
    }
}

void print_usage(const char* bin_name) {
	std::printf("Usage: %s [INPUT_FILE] [OUTPUT_FILE]\n", bin_name);
}

void send_bin_name_and_args_to_cout(const int argc, char** argv) {
	std::cout << argv[0] << ':';
	for(int i = 1; i < argc; i++) {
		std::cout << " '" << argv[i] << '\'';
		if(i == argc - 1) {
			std::cout << ':';
		}
	}
	std::cout << ' ';
}

bool path_exists(const std::filesystem::path& path) {
	return std::ifstream(path).is_open();
}

size_t get_size(const std::filesystem::path& path) {
	std::ifstream file { path, std::ios::binary };
	file.seekg(0, std::ios::end);
	return file.tellg();
}

bool test_size(const std::filesystem::path& path) {
	if(get_size(path) < (Magic::Events::Results::Auto::Record::file_header.size() + Magic::Events::Results::Auto::Record::size)){
		return false;
	}

	return true;
}

bool has_header(const std::filesystem::path& path) {
	std::ifstream file { path, std::ios::in | std::ios::binary };
	if(file.is_open() == false) {
		return false;
	}
	std::array<uint8_t, 128> buf;
	file.read(reinterpret_cast<char*>(buf.data()), buf.size());

	return buf == Magic::Events::Results::Auto::Record::file_header;
}

size_t get_misalignment(const std::filesystem::path& path) {
	return (get_size(path) - Magic::Events::Results::Auto::Record::file_header.size()) % Magic::Events::Results::Auto::Record::size;
}

bool is_aligned(const std::filesystem::path& path) {
	if(get_misalignment(path) != 0) {
		return false;
	}

	return true;
}

int do_work(const std::filesystem::path& input_path, std::ostream& os) {
	const size_t wished_num_of_records { (get_size(input_path) - Magic::Events::Results::Auto::Record::file_header.size()) / Magic::Events::Results::Auto::Record::size };
	std::ifstream input_file { input_path, std::ios::in | std::ios::binary };
	if(input_file.is_open() == false) {
		std::cout << "Could not open the input file: '" << input_path.string() << '\'' << std::endl;
		return -1;
	}
	input_file.seekg(Magic::Events::Results::Auto::Record::file_header.size(), std::ios::beg);
	std::array<uint8_t, Magic::Events::Results::Auto::Record::size> record_serialized;

	for(size_t i = 0; input_file.read(reinterpret_cast<char*>(record_serialized.data()), record_serialized.size()); i++) {
		std::array<uint8_t, sizeof(Magic::Events::Results::Auto::Timeval::T_RawData)> timeval_serialized;
		std::copy(record_serialized.begin(), record_serialized.begin() + timeval_serialized.size(), timeval_serialized.begin());
		std::array<uint8_t, sizeof(Magic::Events::Results::Auto::Point::T_RawData)> point_serialized;
		std::copy(record_serialized.begin() + timeval_serialized.size(), record_serialized.end(), point_serialized.begin());
		const ns::Record record {
			Magic::Events::Results::Auto::Timeval::from_raw_data(timeval_serialized),
			Magic::Events::Results::Auto::Point::from_raw_data(point_serialized)
		};
		const json j = record;

		static bool first { true };
		if(first == true) {
			os << '[';
			first = false;
		} else {
			os << ',' << std::endl;
		}
		os << std::setw(4) << j;
		if(i == wished_num_of_records - 1) {
			os << ']' << std::endl;
		}
	}

	return 0;
}

int main(int argc, char** argv) {
	try {
		if(argc != 2) {
			send_bin_name_and_args_to_cout(argc, argv);
			std::cout << "Invalid input: Needs exactly 2 arguments: number of arguments: " << argc - 1 << std::endl;
			print_usage(argv[0]);
			return EXIT_FAILURE;
		}

		const std::filesystem::path input_path { argv[1] };
		if(path_exists(input_path) == false) {
			send_bin_name_and_args_to_cout(argc, argv);
			std::cout << "Invalid input: specified input file: '" << input_path.string() << "' doesn't exist" << std::endl;
			return EXIT_FAILURE;
		}

		if(test_size(input_path) == false) {
			send_bin_name_and_args_to_cout(argc, argv);
			std::cout << "Invalid input file: '" << input_path.string() << "' too small, must be at least: " << Magic::Events::Results::Auto::Record::file_header.size() + Magic::Events::Results::Auto::Record::size << " bytes\n";
			return EXIT_FAILURE;
		}

		if(has_header(input_path) == false) {
			send_bin_name_and_args_to_cout(argc, argv);
			std::cout << "Invalid input file: '" << input_path.string() << "' doesn't have the 128 byte long correct header\n";
			return EXIT_FAILURE;
		}

		if(is_aligned(input_path) == false) {
			send_bin_name_and_args_to_cout(argc, argv);
			std::cout << "Invalid input file: '" << input_path.string() << "' is misaligned by: " << get_misalignment(input_path) << " bytes\n";
			return EXIT_FAILURE;
		}

		if(do_work(input_path, std::cout) != 0) {
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	} catch(const std::exception& e) {
		send_bin_name_and_args_to_cout(argc, argv);
		std::cout << "ERROR: exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
