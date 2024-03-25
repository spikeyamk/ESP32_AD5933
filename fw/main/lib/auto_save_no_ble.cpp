#include <chrono>
#include <fstream>
#include <thread>

#include <trielo/trielo.hpp>
#include <driver/gpio.h>
#include <esp_littlefs.h>
#include <esp_sleep.h>

#include "util.hpp"
#include "sd_card.hpp"
#include "ad5933/measurement/measurement.hpp"
#include "ad5933/driver/driver.hpp"
#include "ad5933/extension/extension.hpp"
#include "default.hpp"
#include "magic/results/results.hpp"

namespace AutoSaveNoBLE {
    void button_check(std::jthread& worker, bool& done, bool& deep_sleep_blocked) {
		Util::Mode::get_instance();
		Trielo::trielo<Util::init_exit_auto_save_no_ble_button>();

		deep_sleep_blocked = true;
		for(size_t i = 0, timeout_ms = 5'000; i <= timeout_ms && gpio_get_level(Util::button) == 0; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			if(i == timeout_ms) {
				done = true;
				worker.join();
				SD_Card::deinit();
				Util::mode_status = Util::Status::BLE;
				esp_restart();
			}
		}

		while(1) {
			deep_sleep_blocked = false;
			auto status { Util::Mode::get_instance().read() };
			deep_sleep_blocked = true;
			for(size_t i = 0, timeout_ms = 5'000; i <= timeout_ms && gpio_get_level(Util::button) == 0; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				if(i == timeout_ms) {
					done = true;
					worker.join();
					SD_Card::deinit();
					Util::mode_status = Util::Status::BLE;
					esp_restart();
				}
			}
		}
    }

	void work(bool& done, const std::chrono::microseconds sleep_us, bool& deep_sleep_blocked) {
		Util::Blinky::get_instance().start(Util::Blinky::Mode::Burst, std::chrono::milliseconds(2'000));
		std::cout << "BLE::Server::Actions::Auto::start_saving sleep_us: " << sleep_us << std::endl;

		AD5933::Driver driver { I2C::Bus::get_instance().active_device_handles.at(AD5933::SLAVE_ADDRESS) };
		AD5933::Extension extension { driver };

		if(Trielo::trielo<SD_Card::init>(Trielo::OkErrCode(0)) != 0) {
			Trielo::trielo<SD_Card::deinit>(Trielo::OkErrCode(0));
			std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
			if(Trielo::trielo<SD_Card::init>(Trielo::OkErrCode(0)) != 0) {
				return;
			}
		}

		while(done == false) {
			size_t bytes_total;
			size_t bytes_free;
			
			if(Trielo::trielo<esp_littlefs_sdmmc_info>(Trielo::OkErrCode(ESP_OK), &SD_Card::card, &bytes_total, &bytes_free) != ESP_OK) {
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
						Trielo::trielo<std::fflush>(Trielo::OkErrCode(0), file);
						Trielo::trielo<std::fclose>(Trielo::OkErrCode(0), file);
					}

					ad5933.set_command(AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode);
				}
			};

			Scoped_FILE file {
				.ad5933 = extension,
				.file = Trielo::trielo<std::fopen>(Trielo::FailErrCode<FILE*>(nullptr), record_filepath.data(), "w")
			};

			if(file.file == nullptr) {
				std::cout << "ERROR: BLE::Actions::Auto::start_saving: Could not open file at first try\n";
				return;
			}

			driver.program_all_registers(Default::config.to_raw_array());
			for(size_t i = 0; i < Default::max_file_size && done == false; i += sizeof(Magic::Results::Auto::Record::Entry)) {
				extension.reset();
				extension.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StandbyMode);
				if(deep_sleep_blocked) {
					std::this_thread::sleep_for(sleep_us);
				} else {
					esp_sleep_enable_timer_wakeup(sleep_us.count());
				}
				extension.set_command(AD5933::Masks::Or::Ctrl::HB::Command::InitStartFreq);
				extension.set_command(AD5933::Masks::Or::Ctrl::HB::Command::StartFreqSweep);
				do {
					while(extension.has_status_condition(AD5933::Masks::Or::Status::ValidData) == false) {
						if(deep_sleep_blocked) {
							std::this_thread::sleep_for(sleep_us);
						} else {
							esp_sleep_enable_timer_wakeup(1'000);
						}
					}
					const auto raw_data { extension.read_impe_data() };
					if(raw_data.has_value() && extension.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete)) {
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
							std::cout << "ERROR: BLE::Actions::Auto::start_saving: Could not fflush the file for writing in the loop: std::fflush(file.file) != 0\n";
							return;
						}
					}
					extension.set_command(AD5933::Masks::Or::Ctrl::HB::Command::IncFreq);
				} while(extension.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete) == false);
			}
		}
	}

    void run() {
		bool done { false };
		bool deep_sleep_blocked { false };
		std::jthread worker(work, std::ref(done), std::chrono::microseconds(100'000), std::ref(deep_sleep_blocked));
		button_check(worker, done, deep_sleep_blocked);
    }
}