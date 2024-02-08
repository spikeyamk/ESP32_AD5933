#include <iostream>
#include <span>
#include <array>
#include <cstdint>
#include <cstdio>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <esp_vfs_fat.h>

#include "ble/server/server.hpp"
#include "trielo/trielo.hpp"
#include "esp_system.h"
#include "util.hpp"
#include "magic/events/results.hpp"
#include "magic/events/commands.hpp"
#include "magic/packets/outcoming.hpp"
#include "sd_card.hpp"

namespace BLE {
	namespace Events {
		namespace Debug {
		}

		namespace FreqSweep {
		};
	}

	namespace Actions {
		void turn_on() {
			Util::Blinky::get_instance().start(std::chrono::milliseconds(100));
			Server::run();
		};

		void advertise() {
			Util::Blinky::get_instance().set_blink_time(std::chrono::milliseconds(200));
			Server::advertise();
		};

		void advertising() {
		};

		void set_connected_blink_time() {
			Util::Blinky::get_instance().set_blink_time(std::chrono::milliseconds(500));
		};

		void disconnect() {

		};

		void unexpected() {
			Util::Blinky::get_instance().stop();
			Server::stop();
		}

		void sleep() {
			Util::Blinky::get_instance().stop();
			Trielo::trielo<SD_Card::deinit>();
			Server::stop();
		}

		namespace Debug {
			static const char *namespace_name = "Debug::";
			void start() {
			}

			void end() {
			}

			void dump(Server::Sender &sender, AD5933::Extension &ad5933) {
				std::unique_lock lock(sender.mutex);
				const auto ret { ad5933.driver.dump_all_registers() };
				if(ret.has_value()) {
					const Magic::Events::Results::Debug::Dump dump_event { ret.value() };
					const Magic::OutComingPacket<Magic::Events::Results::Debug::Dump> dump_packet { dump_event };
					sender.notify_hid_information(dump_packet);
				}
			}

			void program(AD5933::Extension &ad5933, const Magic::Events::Commands::Debug::Program& program_event) {
				std::for_each(program_event.registers_data.end(), program_event.registers_data.end(), [index = 0](const auto e) mutable {
					if(index % 8 == 0){
						std::printf("\n");
					}
					std::printf("0x%02X, ", e);
					index++;
				});
				std::printf("\n");

				ad5933.driver.block_write_to_register<12, AD5933::RegAddrs::RW::ControlHB>(program_event.registers_data);
			}

			void command(AD5933::Extension &ad5933, const Magic::Events::Commands::Debug::CtrlHB &ctrl_HB_event) {
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command(ctrl_HB_event.register_data));
			}
		}

		namespace FreqSweep {
			static const char *namespace_name = "FreqSweep::";
			//static void configure(bool &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event, boost::sml::back::process<Events::FreqSweep::Private::configure_complete> process_event) {
			void configure(std::atomic<bool> &processing, AD5933::Extension &ad5933, const Magic::Events::Commands::Sweep::Configure &configure_event) {
				processing = true;
				std::for_each(configure_event.registers_data.end(), configure_event.registers_data.end(), [index = 0](const auto e) mutable {
					if(index % 8 == 0){
						std::printf("\n");
					}
					std::printf("0x%02X, ", e);
					index++;
				});
				std::printf("\n");
				ad5933.driver.block_write_to_register<12, AD5933::RegAddrs::RW::ControlHB>(configure_event.registers_data);
				processing = false;
			}

			//static void run(Sender &sender, AD5933::Extension &ad5933, boost::sml::back::process<Events::FreqSweep::Private::sweep_complete> process_event) {
			void run(std::atomic<bool> &processing, Server::Sender &sender, AD5933::Extension &ad5933) {
				std::unique_lock lock(sender.mutex);
				processing = true;
				ad5933.reset();
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode);
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq);
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep);
				do {
					while(ad5933.has_status_condition(AD5933::Masks::Or::Status::ValidData) == false) {
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
					const auto data { ad5933.read_impe_data() };
					if(data.has_value()) {
						const Magic::Events::Results::Sweep::ValidData valid_data_event { data.value() };
						const Magic::OutComingPacket<Magic::Events::Results::Sweep::ValidData> valid_data_packet { valid_data_event };
						sender.notify_hid_information(valid_data_packet);
					}
					ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
				} while(ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete) == false);
				
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				processing = false;
			}

			void end() {
			}
		}

		namespace File {
			void free(Server::Sender &sender) {
				uint64_t bytes_total;
				uint64_t bytes_free;
				if(Trielo::trielo<esp_vfs_fat_info>(Trielo::OkErrCode(ESP_OK), SD_Card::mount_point.data(), &bytes_total, &bytes_free) == ESP_OK) {
					std::cout << "BLE::Actions::File::free: \n\tbytes_total: " << bytes_total << "\n\tbytes_free: " << bytes_free << std::endl;
					const Magic::Events::Results::File::Free file_free_event { .bytes_free = bytes_free, .bytes_total = bytes_total };
					const Magic::OutComingPacket<Magic::Events::Results::File::Free> file_free_packet { file_free_event };
					sender.notify_hid_information(file_free_packet);
				}
			}

			uint64_t get_num_of_files() {
				DIR *dir;
				if((dir = opendir(SD_Card::mount_point.data())) == NULL) {
					return 0;
				}

				struct dirent *entry;
				uint64_t num_of_files = 0;
				while((entry = readdir(dir)) != NULL) {
					if(entry->d_type == DT_REG) {
						num_of_files++;
					}
				}

				closedir(dir);
				return num_of_files;
			}

			void list_count(Server::Sender &sender) {
				try {
					const uint64_t num_of_files = get_num_of_files();
					std::cout << "BLE::Actions::File::list_count: \n\tnum_of_files: " << num_of_files << std::endl;
					const Magic::Events::Results::File::ListCount list_count_event { .num_of_files = num_of_files };
					const Magic::OutComingPacket<Magic::Events::Results::File::ListCount> list_count_packet { list_count_event };
					sender.notify_hid_information(list_count_packet);
				} catch(...) {

				}
			}

			void list(Server::Sender &sender) {
				try {
					DIR *dir;
					if((dir = opendir(SD_Card::mount_point.data())) == NULL) {
						return;
					}

					struct dirent *entry;
					while((entry = readdir(dir)) != NULL) {
						if(entry->d_type == DT_REG) {
							const std::string_view filepath (entry->d_name, sizeof(Magic::T_MaxDataSlice));
							Magic::T_MaxDataSlice tmp_path;
							std::copy(filepath.begin(), filepath.end(), tmp_path.begin());
							const Magic::Events::Results::File::List list_event { .path = tmp_path };
							const Magic::OutComingPacket<Magic::Events::Results::File::List> list_packet { list_event };
							sender.notify_hid_information(list_packet);
						}
					}
				} catch(...) {

				}
			}

			std::optional<uint64_t> get_num_of_bytes(const Magic::Events::Commands::File::Size& event) {
				struct stat st;
				std::array<char, SD_Card::mount_point_prefix.size() + sizeof(Magic::T_MaxDataSlice)> path;
				std::copy(SD_Card::mount_point_prefix.begin(), SD_Card::mount_point_prefix.end(), path.begin());
				std::copy(event.path.begin(), event.path.end(), path.begin() + SD_Card::mount_point_prefix.size());
				if(stat(path.data(), &st) != 0) {
					return std::nullopt;
				}
				return static_cast<uint64_t>(st.st_size);
			}

			void size(const Magic::Events::Commands::File::Size& event, Server::Sender &sender) {
				try {
					const auto num_of_bytes { get_num_of_bytes(event) };
					if(num_of_bytes.has_value() == false) {
						return;
					}
					const Magic::Events::Results::File::Size size_event {
						.num_of_bytes = num_of_bytes.value()
					};
					const Magic::OutComingPacket<Magic::Events::Results::File::Size> size_packet { size_event };
					sender.notify_hid_information(size_packet);
				} catch(...) {

				}
			}

			void remove(const Magic::Events::Commands::File::Remove& event) {
				std::array<char, SD_Card::mount_point_prefix.size() + sizeof(Magic::T_MaxDataSlice)> path;
				std::copy(SD_Card::mount_point_prefix.begin(), SD_Card::mount_point_prefix.end(), path.begin());
				std::copy(event.path.begin(), event.path.end(), path.begin() + SD_Card::mount_point_prefix.size());
				unlink(path.data());
			}

			void download(const Magic::Events::Commands::File::Download& event, Server::Sender &sender) {
				try {
					std::array<char, SD_Card::mount_point_prefix.size() + sizeof(Magic::T_MaxDataSlice)> path;
					std::copy(SD_Card::mount_point_prefix.begin(), SD_Card::mount_point_prefix.end(), path.begin());
					std::copy(event.path.begin(), event.path.end(), path.begin() + SD_Card::mount_point_prefix.size());

					FILE* file = std::fopen(path.data(), "rb");
					if(file == NULL) {
						return;
					}

					Magic::T_MaxDataSlice tmp_slice { 0 };
					while(std::fread(tmp_slice.data(), 1, tmp_slice.size(), file)) {
						const Magic::Events::Results::File::Download download_event { .slice = tmp_slice };
						const Magic::OutComingPacket<Magic::Events::Results::File::Download> download_packet { download_event };
						std::cout << "BLE::Actions::File::download: tmp_slice: \n\t" << std::string_view{ reinterpret_cast<const char*>(tmp_slice.data()), tmp_slice.size() } << std::endl;
						sender.notify_hid_information(download_packet);
						tmp_slice = Magic::T_MaxDataSlice { 0 };
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
					}

					std::fclose(file);
				} catch(...) {

				}
			}

			void upload(const Magic::Events::Commands::File::Upload& event) {

			}
		}
	}

	namespace Guards {
		bool processing(std::atomic<bool> &processing) {
			return !processing.load();
		}

		bool neg_processing(std::atomic<bool> &processing) {
			return processing.load();
		}
	}
}