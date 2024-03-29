#include <thread>
#include <memory>
#include <future>

#include <trielo/trielo.hpp>
#include <Windows.h>

#include "ble_client/child_main.hpp"
#include "ble_client/shm/parent/parent.hpp"
#include "ad5933/config/config.hpp"
#include "ad5933/data/data.hpp"
#include "ble_client/tests/tests.hpp"

namespace BLE_Client {
    namespace Tests {
        namespace Thread {
            namespace FreqSweep {
                template <typename Decay, typename T>
                constexpr bool variant_tester(const T& variant) {
                    bool ret { false };
                    std::visit(
                        [&ret](auto&& decay) {
                            if constexpr(std::is_same_v<Decay, std::decay_t<decltype(decay)>>) {
                                ret = true;
                            } else {
                                std::cout
                                    << "typeid(std::decay_t<decltype(decay)>).name(): "
                                    << typeid(std::decay_t<decltype(decay)>).name()
                                    << std::endl;
                            }
                        },
                        variant
                    );
                    return ret;
                }

                int periodic() {
                    static const boost::filesystem::path self_path {
                        []() {
                            std::basic_string<boost::filesystem::path::value_type> ret;
                            #ifdef _WIN32 // This is needed in order for the path to work on Windows with UTF-16 special characters (Windows uses const wchar_t* for paths) in the path and whitespace otherwise launching the child process will fail
                                ret.resize(MAX_PATH);
                                if(GetModuleFileNameW(NULL, ret.data(), MAX_PATH) == 0) {
                                    std::cout << "ERROR: GUI: GetModuleFileNameW(NULL, ret.data(), MAX_PATH); failed: Could not retrieve self_path\n";
                                    std::exit(EXIT_FAILURE);
                                }
                            #elif __linux__
                                ret.resize(PATH_MAX);
                                ssize_t len = readlink("/proc/self/exe", ret.data(), ret.size() - 1);
                                if(len == -1) {
                                    return std::string("");
                                }
                                ret[len] = '\0';
                            #endif
                            return ret;
                        }()
                    };

                    std::shared_ptr<BLE_Client::SHM::ParentSHM> shm { nullptr };
                    try {
                        shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
                    } catch(const boost::interprocess::interprocess_exception& e) {
                        std::cout << "ERROR: GUI: Failed to open SHM: exception: " << e.what() << std::endl;
                        BLE_Client::SHM::clean(self_path.string());
                        try {
                            shm = std::make_shared<BLE_Client::SHM::ParentSHM>();
                        } catch(const boost::interprocess::interprocess_exception& e) {
                            std::cout << "ERROR: GUI: Failed to open SHM even after cleaning: exception: " << e.what() << std::endl;
                            return -1;
                        }
                    }

                    std::jthread child(child_main);

                    shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::turn_on{});
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if(variant_tester<BLE_Client::StateMachines::Adapter::States::on>(*shm->active_state) == false) {
                        shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
                        return -2;
                    }

                    shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::start_discovery{});
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if(variant_tester<BLE_Client::StateMachines::Adapter::States::discovering>(*shm->active_state) == false) {
                        shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
                        return -3;
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(10'000));

                    const auto device_it {
                        std::find_if(
                            shm->discovery_devices->begin(),
                            shm->discovery_devices->end(),
                            [](const BLE_Client::Discovery::Device& e) {
                                constexpr std::string_view nimble_address {
                                    #ifdef _WIN32
                                    "40:4c:ca:43:11:b2"
                                    #else
                                    "40:4C:CA:43:11:B2"
                                    #endif
                                };

                                return (
                                    e.get_address() == nimble_address
                                    && e.get_connected() == false
                                );
                            }
                        )
                    };

                    if(device_it == shm->discovery_devices->end()) {
                        shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
                        return -4;
                    }

                    shm->cmd.send_adapter(BLE_Client::StateMachines::Adapter::Events::stop_discovery{});

                    std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
                    try {
                        const BLE_Client::StateMachines::Connector::Events::connect connect { device_it->get_address() };
                        shm->cmd.send(connect);
                        std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
                        shm->attach_device(connect);
                    } catch(const std::exception& e) {
                        std::cout << "ERROR: BLE_Client::Tests::Thread::FreqSweep::periodic:: exception: " << e.what() << std::endl;
                        shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
                        return -5;
                    }

                    shm->cmd.send(
                        BLE_Client::StateMachines::Connection::Events::write_body_composition_feature {
                            .index = 0,
                            .event_variant = Magic::Commands::Sweep::Configure {
                                .registers_data = AD5933::Config().to_raw_array()
                            }
                        }
                    );

                    std::promise<int> sweep_run_thread_promise;
                    std::future<int> sweep_run_thread_future { sweep_run_thread_promise.get_future() };

                    std::jthread sweep_run_thread(
                        [&sweep_run_thread_promise](std::stop_token st, std::shared_ptr<BLE_Client::SHM::ParentSHM> shm) {
                            const BLE_Client::StateMachines::Connection::Events::write_body_composition_feature sweep_run { 0, Magic::Commands::Sweep::Run{} };
                            while(st.stop_requested() == false) {
                                shm->cmd.send(sweep_run);
                                const auto rx_payload { shm->active_devices.begin()->measurement->read_for(boost::posix_time::milliseconds{1'000}) };

                                if(rx_payload.has_value() == false) {
                                    std::cout << "BLE_Client::Tests::Thread::sync_scan::sweep_run_thread: rx_payload.has_value() == false: timeout" << std::endl;
                                    sweep_run_thread_promise.set_value_at_thread_exit(-6);
                                }

                                if(variant_tester<Magic::Results::Sweep::ValidData>(rx_payload.value()) == false) {
                                    std::cout << "BLE_Client::Tests::Thread::sync_scan::sweep_run_thread: variant_tester<Magic::Results::Sweep::ValidData>(rx_payload.value()) == false: wrong variant type" << std::endl;
                                    sweep_run_thread_promise.set_value_at_thread_exit(-7);
                                }

                                std::visit([](auto&& e) {
                                    if constexpr(std::is_same_v<std::decay_t<decltype(e)>, Magic::Results::Sweep::ValidData>) {
                                        const AD5933::Data tmp_data { e.real_imag_registers_data };
                                        std::cout
                                            << "BLE_Client::Tests::Thread::Connect::sweep_run_thread: tmp_data.get_real_data(): "
                                            << tmp_data.get_real_data()
                                            << " tmp_data.get_imag_data(): "
                                            << tmp_data.get_imag_data()
                                            << std::endl;
                                    }
                                }, rx_payload.value());
                            }

                            sweep_run_thread_promise.set_value_at_thread_exit(0);
                        },
                        shm
                    );

                    std::getc(stdin);
                    sweep_run_thread.request_stop();

                    if(sweep_run_thread_future.get() != 0) {
                        return sweep_run_thread_future.get();
                    }
                    shm->cmd.send(BLE_Client::StateMachines::Connection::Events::write_body_composition_feature{ 0, Magic::Commands::Sweep::End{} });

                    shm->cmd.send_killer(BLE_Client::StateMachines::Killer::Events::kill{});
                    return 0;
                }
            }
        }
    }
}

int main() {
    return Trielo::trielo<BLE_Client::Tests::Thread::FreqSweep::periodic>(Trielo::OkErrCode(0));
}