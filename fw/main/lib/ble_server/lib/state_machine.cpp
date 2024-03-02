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

#include "ble/server/server.hpp"
#include "esp_system.h"
#include "util.hpp"
#include "magic/events/results.hpp"
#include "magic/events/commands.hpp"
#include "magic/packets/outcoming.hpp"
#include "sd_card.hpp"
#include "ad5933/config/config.hpp"
#include "ad5933/data/data.hpp"
#include "ad5933/calibration/calibration.hpp"
#include "ad5933/measurement/measurement.hpp"

namespace BLE {
	namespace Actions {
		void turn_on() {
			Util::Blinky::get_instance().start(std::chrono::milliseconds(50));
			Server::run();
		};

		void advertise() {
			Util::Blinky::get_instance().set_blink_time(std::chrono::milliseconds(100));
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
			Util::Blinky::get_instance().stop();
			Server::stop();
		}

		void sleep() {
			Util::Blinky::get_instance().stop();
			Trielo::trielo<SD_Card::deinit>();
			std::this_thread::sleep_for(std::chrono::seconds(3));
			Trielo::trielo<Server::stop>();
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
					const Magic::Events::Results::Debug::Dump dump_event { ret.value() };
					const Magic::OutComingPacket<Magic::Events::Results::Debug::Dump> dump_packet { dump_event };
					sender->notify_hid_information(dump_packet);
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
							const Magic::Events::Results::Sweep::ValidData valid_data_event { data.value() };
							const Magic::OutComingPacket<Magic::Events::Results::Sweep::ValidData> valid_data_packet { valid_data_event };
							sender->notify_hid_information(valid_data_packet);
						}
						ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
					} while(ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete) == false && stop_sources.run);
					
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					processing = false;
				}, sender, std::ref(processing), std::ref(ad5933), std::ref(stop_sources)).detach();
			}

			void end() {
			}
		}

		namespace File {
			void free(std::shared_ptr<Server::Sender> &sender) {
				size_t total_bytes;
				size_t used_bytes;
				if(Trielo::trielo<esp_littlefs_sdmmc_info>(Trielo::OkErrCode(ESP_OK), &SD_Card::card, &total_bytes, &used_bytes) == ESP_OK) {
					std::cout << "BLE::Actions::File::free: \n\ttotal_bytes: " << total_bytes << "\n\tused_bytes: " << used_bytes << std::endl;
					const Magic::Events::Results::File::Free file_free_event { .used_bytes = used_bytes, .total_bytes = total_bytes };
					const Magic::OutComingPacket<Magic::Events::Results::File::Free> file_free_packet { file_free_event };
					sender->notify_hid_information(file_free_packet);
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
				try {
					const uint64_t num_of_files = get_num_of_files();
					std::cout << "BLE::Actions::File::list_count: \n\tnum_of_files: " << num_of_files << std::endl;
					const Magic::Events::Results::File::ListCount list_count_event { .num_of_files = num_of_files };
					const Magic::OutComingPacket<Magic::Events::Results::File::ListCount> list_count_packet { list_count_event };
					sender->notify_hid_information(list_count_packet);
				} catch(...) {

				}
			}

			void list(std::shared_ptr<Server::Sender> &sender) {
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
							sender->notify_hid_information(list_packet);
						}
					}
				} catch(...) {

				}
			}

			std::optional<uint64_t> get_num_of_bytes(const Magic::Events::Commands::File::Size& event) {
				struct stat st;
				std::array<char, SD_Card::mount_point_prefix.size() + Magic::MTU> path { 0 };
				std::copy(SD_Card::mount_point_prefix.begin(), SD_Card::mount_point_prefix.end(), path.begin());
				std::copy(event.path.begin(), event.path.end(), path.begin() + SD_Card::mount_point_prefix.size());
				if(stat(path.data(), &st) != 0) {
					return std::nullopt;
				}
				return static_cast<uint64_t>(st.st_size);
			}

			void size(const Magic::Events::Commands::File::Size& event, std::shared_ptr<Server::Sender> &sender) {
				try {
					const auto num_of_bytes { get_num_of_bytes(event) };
					if(num_of_bytes.has_value() == false) {
						return;
					}
					const Magic::Events::Results::File::Size size_event {
						.num_of_bytes = num_of_bytes.value()
					};
					const Magic::OutComingPacket<Magic::Events::Results::File::Size> size_packet { size_event };
					sender->notify_hid_information(size_packet);
				} catch(...) {

				}
			}

			void remove(const Magic::Events::Commands::File::Remove& event) {
				std::array<char, SD_Card::mount_point_prefix.size() + Magic::MTU> path { 0 };
				std::copy(SD_Card::mount_point_prefix.begin(), SD_Card::mount_point_prefix.end(), path.begin());
				std::copy(event.path.begin(), event.path.end(), path.begin() + SD_Card::mount_point_prefix.size());
				unlink(path.data());
			}

			void download(const Magic::Events::Commands::File::Download& event, std::shared_ptr<Server::Sender> &sender, StopSources& stop_sources) {
				std::thread([](const Magic::Events::Commands::File::Download event, std::shared_ptr<Server::Sender> sender, StopSources& stop_sources) {
					stop_sources.download = true;
					std::array<char, SD_Card::mount_point_prefix.size() + Magic::MTU> path { 0 };
					std::copy(SD_Card::mount_point_prefix.begin(), SD_Card::mount_point_prefix.end(), path.begin());
					std::copy(event.path.begin(), event.path.end(), path.begin() + SD_Card::mount_point_prefix.size());

					FILE* file = std::fopen(path.data(), "rb");
					if(file == NULL) {
						std::cout << "BLE::Actions::File::download: failed to pen file: " << std::string_view{ reinterpret_cast<const char*>(path.data()), path.size() } << std::endl;
						return;
					}

					Magic::T_MaxDataSlice tmp_slice { 0 };

					const auto start { std::chrono::high_resolution_clock::now() };
					while(stop_sources.download) {
						for(size_t i = 0, stopper = 100; stop_sources.download && i < stopper; i++) {
							if(std::fread(tmp_slice.data(), 1, tmp_slice.size(), file) == 0) {
								stop_sources.download = false;
								break;
							}
							const Magic::Events::Results::File::Download download_event { .slice = tmp_slice };
							const Magic::OutComingPacket<Magic::Events::Results::File::Download> download_packet { download_event };
							sender->notify_hid_information(download_packet);
							tmp_slice = Magic::T_MaxDataSlice { 0 };
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}

					std::fclose(file);
					const auto finish { std::chrono::high_resolution_clock::now() };
					std::cout << "BLE::Actions::download: finish - start: " << std::chrono::duration_cast<std::chrono::seconds>(finish - start) << std::endl;
					std::cout << "BLE::Actions::download: estimated speed: " << get_num_of_bytes(Magic::Events::Commands::File::Size{ event.path }).value_or(0) / std::chrono::duration_cast<std::chrono::seconds>(finish - start).count() << " bytes/s\n";

				}, event, sender, std::ref(stop_sources)).detach();
			}

			void upload(const Magic::Events::Commands::File::Upload& event) {

			}

			void format(std::shared_ptr<Server::Sender> &sender) {
				const Magic::Events::Results::File::Format format_result {
					.status = Trielo::trielo<SD_Card::format>(Trielo::OkErrCode(0)) == 0 ? true : false,
				};
				const Magic::OutComingPacket<Magic::Events::Results::File::Format> format_result_packet {
					format_result
				};
				sender->notify_hid_information(format_result_packet);
			}

			void create_test_files(std::shared_ptr<Server::Sender> &sender) {
				const Magic::Events::Results::File::CreateTestFiles create_test_files_result {
					.status = Trielo::trielo<SD_Card::create_test_files>(Trielo::OkErrCode(0)) == 0,
				};

				try {
					static char junk[] { "ratatatatata\nabba\nbaba\nhaha\n" };
					std::ofstream(Auto::get_record_file_name_zero_terminated().data(), std::ios::out) << junk;
				} catch(...) {

				}

				Trielo::trielo<SD_Card::print_test_files>();
				const Magic::OutComingPacket<Magic::Events::Results::File::CreateTestFiles> create_test_files_packet {
					create_test_files_result
				};
				sender->notify_hid_information(create_test_files_packet);
			}
		}

		namespace Auto {
			static constexpr AD5933::Config default_config {};
			static constexpr std::array<AD5933::Calibration<float>, 3> default_calibration {
				AD5933::Calibration<float> { 9806023.0f, -1.2272441387176514f },
				AD5933::Calibration<float> { 9813488.0f, -1.2278409004211426f },
				AD5933::Calibration<float> { 9811605.0f, -1.2277723550796509f },
			};

			std::array<char, 28> get_record_file_name_zero_terminated() {
				const time_t time { std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now()) };
				const tm* tm { std::localtime(&time) };
				std::array<char, 28> ret;
				std::strftime(ret.data(), sizeof(ret), "/sdcard/%Y-%m-%d-%H_%M_%S", tm);
				return ret;
			}

			void start_saving(const Magic::Events::Commands::Auto::Save& event, std::shared_ptr<Server::Sender>& sender, StopSources& stop_sources, AD5933::Extension &ad5933) {
				std::jthread t1([sender, &ad5933, sleep_ms = std::chrono::milliseconds(1 + event.tick_ms)](StopSources& stop_sources) {
					std::cout << "BLE::Server::Actions::Auto::start_saving sleep_ms: " << sleep_ms << std::endl;
					stop_sources.save = true;

					size_t bytes_total;
					size_t bytes_free;
					
					if(Trielo::trielo<esp_littlefs_sdmmc_info>(Trielo::OkErrCode(ESP_OK), &SD_Card::card, &bytes_total, &bytes_free) != ESP_OK) {
						return;
					}

					if(bytes_free < sizeof(Magic::Events::Results::Auto::Timeval::T_RawData) + sizeof(Magic::Events::Results::Auto::Point::T_RawData)) {
						// We should never get here, this is bad
						std::cout << "ERROR: BLE::Actions::Auto::start_saving: filesystem ain't got no space in it: bytes_free: " << bytes_free << std::endl;
						return;
					}

					// This is so bad, I don't even know what to say
					std::array<char, 28U> record_filepath { 0 };
					for(uint8_t i = 0, stopper = 16;
						std::ifstream((record_filepath = get_record_file_name_zero_terminated()).data()).is_open() && i <= stopper;
						i++
					) {
						if(i == stopper) {
							// Removes the record file if it exists we should never get here
							FILE* file_to_remove = Trielo::trielo<std::fopen>(Trielo::FailErrCode<FILE*>(nullptr), record_filepath.data(), "r");
							if(file_to_remove != nullptr) {
								Trielo::trielo<std::fclose>(Trielo::OkErrCode(0), file_to_remove);
								Trielo::trielo<unlink>(Trielo::OkErrCode(0), record_filepath.data());
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
								std::fclose(file);
							}

							ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
						}
					};

					Scoped_FILE file {
						.ad5933 = ad5933,
						.file = Trielo::trielo<std::fopen>(Trielo::FailErrCode<FILE*>(nullptr), record_filepath.data(), "w")
					};

					if(file.file == nullptr) {
						std::cout << "ERROR: BLE::Actions::Auto::start_saving: Could not open file at first try\n";
						return;
					}

					ad5933.driver.program_all_registers(default_config.to_raw_array());
					while(stop_sources.save == true) {
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
								const auto now { std::chrono::high_resolution_clock::now() };
								const auto since_epoch { now.time_since_epoch() };
								const auto seconds { std::chrono::duration_cast<std::chrono::seconds>(since_epoch) };
								const auto useconds { std::chrono::duration_cast<std::chrono::microseconds>(since_epoch) };
								const Magic::Events::Results::Auto::Timeval timeval {
									.tv = {
										.tv_sec = static_cast<time_t>(seconds.count()),
										.tv_usec = static_cast<suseconds_t>(useconds.count() - std::chrono::duration_cast<std::chrono::microseconds>(seconds).count())
									}
								};

								const time_t time { std::chrono::high_resolution_clock::to_time_t(now) };
								const tm* tm { std::localtime(&time) };
								std::array<char, 20> time_log { 0 };
								std::strftime(time_log.data(), sizeof(time_log), "%Y-%m-%d-%H_%M_%S", tm);
								std::cout << "BLE::Actions::Auto::Save: " << time_log.data() << std::endl;

								const auto timeval_serialized { timeval.to_raw_data() };
								const size_t timeval_written_size = std::fwrite(timeval_serialized.data(), sizeof(timeval_serialized[0]), timeval_serialized.size(), file.file);
								if(timeval_written_size != timeval_serialized.size()) {
									std::cout << "ERROR: BLE::Actions::Auto::start_saving: Could not open file for writing in the loop: timeval_written_size != timeval_serialized.size(): timeval_written_size: " << timeval_written_size << std::endl;
									return;
								}

								const AD5933::Data data { raw_data.value() };
								const AD5933::Measurement measurement { data, default_calibration[2] };
								const Magic::Events::Results::Auto::Point point {
									.status = static_cast<uint8_t>(Magic::Events::Results::Auto::Point::Status::Valid),
									.config = static_cast<uint8_t>(Magic::Events::Results::Auto::Point::Config::Default),
									.impedance = measurement.get_magnitude(),
									.phase = measurement.get_phase(),
								};
								const auto point_serialized { point.to_raw_data() };
								const size_t point_written_size = std::fwrite(point_serialized.data(), sizeof(point_serialized[0]), point_serialized.size(), file.file);
								if(point_written_size != point_serialized.size()) {
									std::cout << "ERROR: BLE::Actions::Auto::start_saving: Could not open file for writing in the loop: point_written_size != point_serialized.size(): point_written_size: " << point_written_size << std::endl;
									return;
								}
							}
							ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
						} while(ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete) == false);
					}
				}, std::ref(stop_sources));
				t1.detach();
			}

			void start_sending(const Magic::Events::Commands::Auto::Send& event, std::shared_ptr<Server::Sender>& sender, StopSources& stop_sources, AD5933::Extension &ad5933) {
				std::jthread t1([sender, &ad5933, sleep_ms = std::chrono::milliseconds(1 + event.tick_ms)](StopSources& stop_sources) {
					std::cout << "BLE::Server::Actions::Auto::start_sending sleep_ms: " << sleep_ms << std::endl;
					stop_sources.send = true;
					ad5933.driver.program_all_registers(default_config.to_raw_array());
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
								const AD5933::Measurement measurement { data, default_calibration[2] };
								const Magic::Events::Results::Auto::Point point {
									.status = static_cast<uint8_t>(Magic::Events::Results::Auto::Point::Status::Valid),
									.impedance = measurement.get_magnitude(),
									.phase = measurement.get_phase(),
								};
								const Magic::T_OutComingPacket<Magic::Events::Results::Auto::Point, 11> point_packet { point };
								if(sender->notify_body_composition_measurement(point_packet) == false) {
									ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
									return;
								}
							}
							ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
						} while(ad5933.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete) == false);
					}
					ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
				}, std::ref(stop_sources));
				t1.detach();
			}

			void stop_saving(StopSources& stop_sources) {
				stop_sources.save = false;
			}

			void stop_sending(StopSources& stop_sources) {
				stop_sources.send = false;
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