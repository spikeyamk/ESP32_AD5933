#include <thread>

#include "ble_client/child_main.hpp"
#include "ble_client/shm/parent/parent.hpp"

#include "ble_client/tests/tests.hpp"

namespace BLE_Client {
    namespace Tests {
        namespace Thread {
            namespace Connect {
                /*
                int sync_scan() {
                    std::jthread child(child_main);
                    return -1;
                }
                */

                int async_scan() {
                    std::jthread child(child_main);
                    return -1;
                }
            }

            namespace Debug {
                int dump() {
                    std::jthread child(child_main);
                    return -1;
                }

                int program_and_dump() {
                    std::jthread child(child_main);
                    return -1;
                }
            }

            namespace FreqSweep {
                /*
                int periodic() {
                    std::jthread child(child_main);
                    return -1;
                }
                */
            }

            namespace Auto {
                int send() {
                    std::jthread child(child_main);
                    return -1;
                }
            }

            namespace File {
                int free() {
                    std::jthread child(child_main);
                    return -1;
                }

                int list_count() {
                    std::jthread child(child_main);
                    return -1;
                }

                int list() {
                    std::jthread child(child_main);
                    return -1;
                }

                int size() {
                    std::jthread child(child_main);
                    return -1;
                }

                int remove() {
                    std::jthread child(child_main);
                    return -1;
                }

                int download() {
                    std::jthread child(child_main);
                    return -1;
                }
            }
        }

        namespace Interprocess {
            namespace Connect {
                int sync_scan() {
                    return -1;
                }

                int async_scan() {
                    return -1;
                }
            }

            namespace Debug {
                int dump() {
                    return -1;
                }

                int program_and_dump() {
                    return -1;
                }

            }

            namespace File {
                int free() {
                    return -1;
                }

                int list_count() {
                    return -1;
                }

                int list() {
                    return -1;
                }

                int size() {
                    return -1;
                }

                int remove() {
                    return -1;
                }

                int download() {
                    return -1;
                }
            }
        }
    }
}