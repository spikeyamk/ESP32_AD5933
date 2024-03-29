#pragma once

namespace BLE_Client {
    namespace Tests {
        namespace Thread {
            namespace Connect {
                int sync_scan();
                int async_scan();
            }

            namespace Debug {
                int dump();
                int program_and_dump();
            }

            namespace FreqSweep {
                //int periodic();
            }

            namespace Auto {
                int send();
            }

            namespace File {
                int free();
                int list_count();
                int list();
                int size();
                int remove();
                int download();
            }
        }

        namespace Interprocess {
            namespace Connect {
                int sync_scan();
                int async_scan();
            }

            namespace Debug {
                int dump();
                int program_and_dump();
            }

            namespace File {
                int free();
                int list_count();
                int list();
                int size();
                int remove();
                int download();
            }
        }
    }
}