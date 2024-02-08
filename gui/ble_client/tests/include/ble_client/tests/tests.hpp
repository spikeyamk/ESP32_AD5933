#pragma once

namespace BLE_Client {
    namespace Tests {
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