#include <iostream>
#include <span>
#include <array>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <fstream>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <esp_littlefs.h>
#include <trielo/trielo.hpp>
#include <driver/gpio.h>

#include "ble_server/server.hpp"
#include "esp_system.h"
#include "util.hpp"
#include "sd_card.hpp"
#include "default.hpp"
#include "ad5933/data/data.hpp"
#include "ad5933/measurement/measurement.hpp"

namespace BLE {
	namespace Actions {
		void turn_on() {
			TRIELO_EQ(Util::Blinky::get_instance().start(Util::Blinky::Mode::Single, std::chrono::milliseconds(50)), true);
			if(Trielo::trielo<SD_Card::init>(Trielo::Success(0)) != 0) {
				Trielo::trielo<SD_Card::deinit>(Trielo::Success(0));
				std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
				Trielo::trielo<SD_Card::init>(Trielo::Success(0));
			}
			Server::run();
		};

		void advertise() {
			static auto auto_save_no_ble_button_enterer { std::thread([]() {
				Util::Mode::get_instance();
				Util::init_enter_auto_save_no_ble_button();

				for(size_t i = 0, timeout_ms = 5'000; i <= timeout_ms && gpio_get_level(Util::button) == 0; i++) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					if(i == timeout_ms) {
						Util::mode_status = Util::Status::BLE;
						SD_Card::deinit();
						esp_restart();
					}
				}

				while(1) {
					auto status { Util::Mode::get_instance().read() };
					for(size_t i = 0, timeout_ms = 5'000; i <= timeout_ms && gpio_get_level(Util::button) == 0; i++) {
						if(i == timeout_ms) {
							Util::Blinky::get_instance().stop();
							Trielo::trielo<SD_Card::deinit>();
							Trielo::trielo<Server::stop>();
							Util::mode_status = status;
							esp_restart();
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
				}
			}) };

			TRIELO_VOID(Util::Blinky::get_instance().set_blink_time(std::chrono::milliseconds(100)));
			Server::advertise();
		};

		void advertising() {
		};

		void set_connected_blink_time() {
			Util::Blinky::get_instance().set_blink_time(std::chrono::milliseconds(1'000));
		};

		void disconnect(StopSources& stop_sources) {
			stop_sources.download = false;
			stop_sources.run = false;
			stop_sources.save = false;
			stop_sources.send = false;
		};

		void unexpected() {
			sleep();
		}

		void sleep() {
			Trielo::trielo<Util::deinit_enter_auto_save_no_ble_button>();
			TRIELO_EQ(Util::Blinky::get_instance().stop(), true);
			Trielo::trielo<SD_Card::deinit>();
			Trielo::trielo<Server::stop>();
			Util::init_wakeup_button(Util::Status::BLE);
		}

		namespace Debug {
			void start() {
			}

			void end() {
			}

			void dump(std::shared_ptr<Server::Sender> &sender, AD5933::Extension &ad5933) {
				std::unique_lock lock(sender->mutex);
				const auto ret { ad5933.driver.dump_all_registers() };
				if(ret.has_value()) {
					const Magic::Results::Debug::Dump dump_event { ret.value() };
					sender->notify_hid_information(dump_event);
				}
			}

			void program(AD5933::Extension &ad5933, const Magic::Commands::Debug::Program& program_event) {
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

			void command(AD5933::Extension &ad5933, const Magic::Commands::Debug::CtrlHB &ctrl_HB_event) {
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command(ctrl_HB_event.register_data));
			}
		}

		namespace FreqSweep {
			//static void configure(bool &processing, AD5933::Extension &ad5933, const Events::FreqSweep::configure &configure_event, boost::sml::back::process<Events::FreqSweep::Private::configure_complete> process_event) {
			void configure(std::atomic<bool> &processing, AD5933::Extension &ad5933, const Magic::Commands::Sweep::Configure &configure_event) {
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

				const gpio_num_t rcal_mux_pin { GPIO_NUM_20 };
				
				if(configure_event.calen) {
					Trielo::trielo<gpio_reset_pin>(Trielo::Success(ESP_OK), rcal_mux_pin);
					Trielo::trielo<gpio_set_direction>(Trielo::Success(ESP_OK), rcal_mux_pin, GPIO_MODE_OUTPUT);
					Trielo::trielo<gpio_set_pull_mode>(Trielo::Success(ESP_OK), rcal_mux_pin, GPIO_PULLUP_PULLDOWN);
					Trielo::trielo<gpio_set_level>(Trielo::Success(ESP_OK), rcal_mux_pin, 1);
				} else {
					Trielo::trielo<gpio_reset_pin>(Trielo::Success(ESP_OK), rcal_mux_pin);
					Trielo::trielo<gpio_set_direction>(Trielo::Success(ESP_OK), rcal_mux_pin, GPIO_MODE_OUTPUT);
					Trielo::trielo<gpio_set_pull_mode>(Trielo::Success(ESP_OK), rcal_mux_pin, GPIO_PULLDOWN_ONLY);
					Trielo::trielo<gpio_pulldown_en>(Trielo::Success(ESP_OK), rcal_mux_pin);
					Trielo::trielo<gpio_set_level>(Trielo::Success(ESP_OK), rcal_mux_pin, 0);
				}

				processing = false;
			}

			//static void run(Sender &sender, AD5933::Extension &ad5933, boost::sml::back::process<Events::FreqSweep::Private::sweep_complete> process_event) {
			void run(std::atomic<bool> &processing, std::shared_ptr<Server::Sender> &sender, AD5933::Extension &ad5933, StopSources& stop_sources) {
				std::thread([](std::shared_ptr<Server::Sender> sender, std::atomic<bool> &processing, AD5933::Extension &ad5933, StopSources& stop_sources) {
					stop_sources.run = true;
					std::unique_lock lock(sender->mutex);
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
							const Magic::Results::Sweep::ValidData valid_data_event { data.value() };
							for(size_t i = 0, stopper = 100; sender->notify_hid_information(valid_data_event) == false; i++) {
								if(i == stopper) {
									std::cout << "BLE::Actions::FreqSweep: i == stopper\n";
									ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
									return;
								}
								std::this_thread::sleep_for(std::chrono::milliseconds(1));
							}
						}
						if(ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete)) {
							break;
						}
						ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
					} while(stop_sources.run);
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					processing = false;
				}, sender, std::ref(processing), std::ref(ad5933), std::ref(stop_sources)).detach();
			}

			void end(AD5933::Extension &ad5933, std::shared_ptr<Server::Sender> &sender, StopSources& stop_sources) {
				ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
				//sender->release();
				stop_sources.run = false;
			}
		}

		namespace File {
			void free(std::shared_ptr<Server::Sender> &sender) {
				size_t total_bytes { 0 };
				size_t used_bytes { 0 };
				if(Trielo::trielo<esp_littlefs_sdmmc_info>(Trielo::Success(ESP_OK), &SD_Card::card, &total_bytes, &used_bytes) == ESP_OK) {
					std::cout << "BLE::Actions::File::free: \n\ttotal_bytes: " << total_bytes << "\n\tused_bytes: " << used_bytes << std::endl;
					const Magic::Results::File::Free file_free_event { .used_bytes = used_bytes, .total_bytes = total_bytes };
					sender->notify_hid_information(file_free_event);
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

			void list_count(std::shared_ptr<Server::Sender> &sender) {
				std::thread([](std::shared_ptr<Server::Sender> sender) {
					const uint64_t num_of_files = get_num_of_files();
					std::cout << "BLE::Actions::File::list_count: \n\tnum_of_files: " << num_of_files << std::endl;
					const Magic::Results::File::ListCount list_count_event { .num_of_files = num_of_files };
					for(size_t i = 0, stopper = 100; sender->notify_hid_information(list_count_event) == false; i++) {
						if(i == stopper) {
							std::cout << "BLE::Actions::File::list_count: i == stopper\n";
							return;
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}

				}, sender).detach();
			}

			void list(std::shared_ptr<Server::Sender> &sender) {
				std::thread([](std::shared_ptr<Server::Sender> sender) {
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
							const Magic::Results::File::List list_event { .path = tmp_path };
							for(size_t i = 0, stopper = 100; sender->notify_hid_information(list_event) == false; i++) {
								if(i == stopper) {
									std::cout << "BLE::Actions::File::list: i == stopper\n";
									return;
								}
								std::this_thread::sleep_for(std::chrono::milliseconds(1));
							}
						}
					}
				}, sender).detach();
			}

			std::optional<uint64_t> get_num_of_bytes(const Magic::Commands::File::Size& event) {
				struct stat st;
				std::array<char, SD_Card::mount_point_prefix.size() + Magic::MTU> path { 0 };
				std::copy(SD_Card::mount_point_prefix.begin(), SD_Card::mount_point_prefix.end(), path.begin());
				std::copy(event.path.begin(), event.path.end(), path.begin() + SD_Card::mount_point_prefix.size());
				if(stat(path.data(), &st) != 0) {
					return std::nullopt;
				}
				return static_cast<uint64_t>(st.st_size);
			}

			void size(const Magic::Commands::File::Size& event, std::shared_ptr<Server::Sender> &sender) {
				const auto num_of_bytes { get_num_of_bytes(event) };
				if(num_of_bytes.has_value() == false) {
					return;
				}
				const Magic::Results::File::Size size_event {
					.num_of_bytes = num_of_bytes.value()
				};
				sender->notify_hid_information(size_event);
			}

			void remove(const Magic::Commands::File::Remove& event, std::shared_ptr<Server::Sender> &sender) {
				std::array<char, SD_Card::mount_point_prefix.size() + Magic::MTU> path { 0 };
				std::copy(SD_Card::mount_point_prefix.begin(), SD_Card::mount_point_prefix.end(), path.begin());
				std::copy(event.path.begin(), event.path.end(), path.begin() + SD_Card::mount_point_prefix.size());
				const Magic::Results::File::Remove remove_event { .status = Trielo::trielo<unlink>(Trielo::Success(0), path.data()) == 0 };
				sender->notify_hid_information(remove_event);
			}

			void download(const Magic::Commands::File::Download& event, std::shared_ptr<Server::Sender> &sender, StopSources& stop_sources) {
				std::thread([](const Magic::Commands::File::Download event, std::shared_ptr<Server::Sender> sender, StopSources& stop_sources) {
					stop_sources.download = true;
					std::array<char, SD_Card::mount_point_prefix.size() + Magic::MTU> path { 0 };
					std::copy(SD_Card::mount_point_prefix.begin(), SD_Card::mount_point_prefix.end(), path.begin());
					std::copy(event.path.begin(), event.path.end(), path.begin() + SD_Card::mount_point_prefix.size());

					FILE* file = std::fopen(path.data(), "rb");
					if(file == NULL) {
						std::cout << "BLE::Actions::File::download: failed to open file: " << std::string_view{ reinterpret_cast<const char*>(path.data()), path.size() } << std::endl;
						return;
					}

					Magic::T_MaxDataSlice tmp_slice { 0 };
					const auto start { std::chrono::high_resolution_clock::now() };
					while(stop_sources.download && std::fread(tmp_slice.data(), 1, tmp_slice.size(), file) != 0) {
						const Magic::Results::File::Download download_event { .slice = tmp_slice };
						for(size_t i = 0, stopper = 100; sender->notify_hid_information(download_event) == false; i++) {
							if(i == stopper) {
								std::cout << "BLE::Actions::File::download: i == stopper\n";
								return;
							}
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
						}
					}

					std::fclose(file);
					const auto finish { std::chrono::high_resolution_clock::now() };
					std::cout << "BLE::Actions::download: finish - start: " << std::chrono::duration_cast<std::chrono::seconds>(finish - start) << std::endl;
					std::cout << "BLE::Actions::download: estimated speed: " << get_num_of_bytes(Magic::Commands::File::Size{ event.path }).value_or(0) / std::chrono::duration_cast<std::chrono::seconds>(finish - start).count() << " bytes/s\n";
				}, event, sender, std::ref(stop_sources)).detach();
			}

			void format(std::shared_ptr<Server::Sender> &sender) {
				std::thread([](std::shared_ptr<Server::Sender> sender) {
					const Magic::Results::File::Format format_event {
						.status = Trielo::trielo<SD_Card::format>(Trielo::Success(0)) == 0,
					};
					sender->notify_hid_information(format_event);
				}, sender).detach();
			}

			void create_test_files(std::shared_ptr<Server::Sender> &sender) {
				std::thread([](std::shared_ptr<Server::Sender> sender) {
					int ret { 0 };
					ret = Trielo::trielo<SD_Card::create_test_file>(Trielo::Success(0), 1 * 1024, "1kB");
					ret = Trielo::trielo<SD_Card::create_test_file>(Trielo::Success(0), 2 * 1024, "2kB");
					ret = Trielo::trielo<SD_Card::create_test_file>(Trielo::Success(0), 4 * 1024, "4kB");
					//Trielo::trielo<SD_Card::create_test_file>(Trielo::Success(0), 8 * 1024, "8kB");
					//Trielo::trielo<SD_Card::create_test_file>(Trielo::Success(0), 16 * 1024, "16kB");
					//Trielo::trielo<SD_Card::create_test_file>(Trielo::Success(0), 32 * 1024, "32kB");
					//Trielo::trielo<SD_Card::create_test_file>(Trielo::Success(0), 64 * 1024, "64kB");
					//Trielo::trielo<SD_Card::create_test_file>(Trielo::Success(0), 128 * 1024, "128kB");

					const Magic::Results::File::CreateTestFiles create_test_files_event {
						.status = ret == 0
					};

					sender->notify_hid_information(create_test_files_event);
				}, sender).detach();
			}

			void end(std::shared_ptr<Server::Sender> &sender, StopSources& stop_sources) {
				stop_sources.download = false;
				//sender->release();
			}
		}

		namespace Auto {
			void start_saving(const Magic::Commands::Auto::Save& event, std::shared_ptr<Server::Sender>& sender, StopSources& stop_sources, AD5933::Extension &ad5933) {
				std::jthread t1([sender, &ad5933, sleep_ms = std::chrono::milliseconds(1 + event.tick_ms)](StopSources& stop_sources) {
					static char buf[BUFSIZ];
					std::cout << "BLE::Server::Actions::Auto::start_saving sleep_ms: " << sleep_ms << std::endl;
					stop_sources.save = true;

					while(stop_sources.save) {
						size_t bytes_total;
						size_t bytes_free;
						
						if(Trielo::trielo<esp_littlefs_sdmmc_info>(Trielo::Success(ESP_OK), &SD_Card::card, &bytes_total, &bytes_free) != ESP_OK) {
							return;
						}

						if(bytes_free < Serde::get_serialized_size<Magic::Results::Auto::Timeval>() + Serde::get_serialized_size<Magic::Results::Auto::Point>()) {
							// We should never get here, this is bad
							std::cout << "ERROR: BLE::Actions::Auto::start_saving: filesystem ain't got no space in it: bytes_free: " << bytes_free << std::endl;
							return;
						}

						// This is so bad, I don't even know what to say
						std::array<char, 28U> record_filepath { 0 };
						for(uint8_t i = 0, stopper = 16;
							std::ifstream((record_filepath = Util::get_record_file_name_zero_terminated()).data()).is_open() && i <= stopper;
							i++
						) {
							if(i == stopper) {
								// Removes the record file if it exists we should never get here
								FILE* file_to_remove = Trielo::trielo<std::fopen>(Trielo::Error<FILE*>(nullptr), record_filepath.data(), "r");
								if(file_to_remove != nullptr) {
									Trielo::trielo<std::fclose>(Trielo::Success(0), file_to_remove);
									Trielo::trielo<unlink>(Trielo::Success(0), record_filepath.data());
								}
							}
							std::cout << "ERROR: BLE::Actions::Auto::start_saving: file: " << record_filepath.data() << " already exits: waiting for std::chrono::seconds(1)\n";
							std::this_thread::sleep_for(std::chrono::seconds(1));
						}

						// I hate this xddd, but it's much better than anything I've come up with so far
						struct Scoped_FILE {
							AD5933::Extension& ad5933;
							FILE* file {  };
							~Scoped_FILE() {
								if(file != nullptr) {
									Trielo::trielo<std::fflush>(Trielo::Success(0), file);
									Trielo::trielo<std::fclose>(Trielo::Success(0), file);
								}

								ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
							}
						};

						Scoped_FILE file {
							.ad5933 = ad5933,
							.file = Trielo::trielo<std::fopen>(Trielo::Error<FILE*>(nullptr), record_filepath.data(), "w")
						};
						
						std::setbuf(file.file, buf);

						if(file.file == nullptr) {
							std::cout << "ERROR: BLE::Actions::Auto::start_saving: Could not open file at first try\n";
							return;
						}

						ad5933.driver.program_all_registers(Default::config.to_raw_array());
						for(size_t i = 0; stop_sources.save == true && i < Default::max_file_size; i += sizeof(Magic::Results::Auto::Record::Entry::T_RawData)) {
							ad5933.reset();
							ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode);
							std::this_thread::sleep_for(sleep_ms);
							ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq);
							ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep);
							do {
								while(ad5933.has_status_condition(AD5933::Masks::Or::Status::ValidData) == false) {
									std::this_thread::sleep_for(std::chrono::milliseconds(1));
								}
								const auto raw_data { ad5933.read_impe_data() };
								if(raw_data.has_value() && ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete)) {
									const time_t time { std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now()) };
									const tm* tm { std::localtime(&time) };
									std::array<char, 20> time_log { 0 };
									std::strftime(time_log.data(), sizeof(time_log), "%Y-%m-%d-%H_%M_%S", tm);
									std::cout << "BLE::Actions::Auto::Save: " << time_log.data() << std::endl;

									const AD5933::Data data { raw_data.value() };
									const AD5933::Measurement measurement { data, Default::calibration.back() };
									const Magic::Results::Auto::Record::Entry entry {
										measurement.get_magnitude(),
										measurement.get_phase(),
									};
									const auto entry_serialized { entry.to_raw_data() };
									const size_t entry_written_size = std::fwrite(entry_serialized.data(), sizeof(entry_serialized[0]), entry_serialized.size(), file.file);
									if(entry_written_size != entry_serialized.size()) {
										std::cout << "ERROR: BLE::Actions::Auto::start_saving: Could not open file for writing in the loop: entry_written_size != entry_serialized.size(): point_written_size: " << entry_written_size << std::endl;
										return;
									}

									if(std::fflush(file.file) != 0) {
										const int fd { fileno(file.file) };
										if(fd == -1) {
											perror("Error getting file descriptor");
											return;
										}

										fsync(fd);
										std::cout << "ERROR: BLE::Actions::Auto::start_saving: Could not fflush the file for writing in the loop: std::fflush(file.file) != 0\n";
										return;
									}
								}
								ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
							} while(ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete) == false);
						}
					}
				}, std::ref(stop_sources));
				t1.detach();
			}

			void start_sending(const Magic::Commands::Auto::Send& event, std::shared_ptr<Server::Sender>& sender, StopSources& stop_sources, AD5933::Extension &ad5933) {
				std::jthread t1([sender, &ad5933, sleep_ms = std::chrono::milliseconds(1 + event.tick_ms)](StopSources& stop_sources) {
					std::cout << "BLE::Server::Actions::Auto::start_sending sleep_ms: " << sleep_ms << std::endl;
					stop_sources.send = true;
					ad5933.driver.program_all_registers(Default::config.to_raw_array());
					while(stop_sources.send == true) {
						ad5933.reset();
						ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode);
						std::this_thread::sleep_for(sleep_ms);
						ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq);
						ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep);
						do {
							while(ad5933.has_status_condition(AD5933::Masks::Or::Status::ValidData) == false) {
								std::this_thread::sleep_for(std::chrono::milliseconds(1));
							}
							const auto raw_data { ad5933.read_impe_data() };
							if(raw_data.has_value() && ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete)) {
								const AD5933::Data data { raw_data.value() };
								const AD5933::Measurement measurement { data, Default::calibration.back() };
								const Magic::Results::Auto::Point point {
									.status = Magic::Results::Auto::Point::Status::Valid,
									.config = Magic::Results::Auto::Point::Config::Default,
									.impedance = measurement.get_magnitude(),
									.phase = measurement.get_phase(),
								};

								for(size_t i = 0, stopper = 100; sender->notify_body_composition_measurement(point) == false; i++) {
									if(i == stopper) {
										std::cout << "BLE::Actions::File::download: i == stopper\n";
										ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
										return;
									}
									std::this_thread::sleep_for(std::chrono::milliseconds(1));
								}
							}
							ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
						} while(ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete) == false);
					}
					ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
				}, std::ref(stop_sources));
				t1.detach();
			}

			void stop_saving(std::shared_ptr<Server::Sender>& sender, StopSources& stop_sources) {
				stop_sources.save = false;
				//sender->release();
			}

			void stop_sending(std::shared_ptr<Server::Sender>& sender, StopSources& stop_sources) {
				stop_sources.send = false;
				//sender->release();
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
