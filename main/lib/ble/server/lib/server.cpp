#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <memory>
#include <atomic>
#include <array>
#include <cstdint>
#include <variant>

#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "modlog/modlog.h"
#include "esp_bt.h"
#include "trielo/trielo.hpp"

#include "ble/server/server.hpp"
#include "magic/packets.hpp"

#include "ad5933/driver/driver.hpp"
#include "ad5933/extension/extension.hpp"
#include "ble/server/sender.hpp"
#include "ble/server/server.hpp"

namespace BLE {
    namespace Server {
        #define APPEARANCE_GENERIC_SENSOR_UUID 0x0540
        #define PROFILE_BODY_COMPOSITION_UUID 0x1014
        #define CHARACTERISTIC_BODY_COMPOSITION_FEATURE_UUID 0x2A9B
        #define CHARACTERISTIC_BODY_COMPOSITION_MEASUREMENT_UUID 0x2A9C
        #define CONFIG_EXAMPLE_IO_TYPE 3

        #define UNIT_ELECTRICAL_RESISTANCE_OHM_UUID 0x272A
        #define UNIT_PLANE_ANGLE_DEGREE_UUID 0x2763
        #define UNIT_PLANE_ANGLE_MINUTE_UUID 0x2764
        #define UNIT_PLANE_ANGLE_RADIAN_UUID 0x2720
        #define UNIT_PLANE_ANGLE_SECOND_UUID 0x2765
        /* */

        const uint8_t SERVICE_BODY_COMPOSITION_UUID[2] = { 0x18, 0x1B };
        static uint8_t own_addr_type;
        static uint16_t body_composition_feature_characteristic_handle;
        static uint16_t conn_handle = 0;
        uint16_t body_composition_measurement_characteristic_handle = 0;
        //static uint16_t body_composition_measurement_characteristic_handle;
        std::atomic<bool> heartbeat_running = false;

        char characteristic_received_value[500] { 0x00 };
    }

    namespace Server {
        Server::Sender sender {};
        AD5933::Driver dummy_driver {};
        AD5933::Extension dummy_extension {dummy_driver};
        std::atomic<bool> processing = false;
        T_StateMachine dummy_state_machine { sender, dummy_extension, processing };
    }

    class Decoder {
    private:
        using T_Variant = std::variant<
            Events::disconnect,
            Events::Debug::start,
            Events::Debug::program,
            Events::Debug::command,
            Events::Debug::dump,
            Events::Debug::end,
            Events::FreqSweep::configure,
            Events::FreqSweep::start,
            Events::FreqSweep::end
        >;
 
        using T_Pair = std::pair<Magic::Packets::Packet_T, T_Variant>;
        static constexpr std::array<T_Pair, 8> jump_info {
            T_Pair{ Magic::Packets::Debug::start, Events::Debug::start{} },
            T_Pair{ Magic::Packets::Debug::dump_all_registers, Events::Debug::dump{} },
            T_Pair{ Magic::Packets::Debug::program_all_registers, Events::Debug::program{} },
            T_Pair{ Magic::Packets::Debug::control_HB_command, Events::Debug::command{} },

            T_Pair{ Magic::Packets::FrequencySweep::configure, Events::FreqSweep::configure{} },
            T_Pair{ Magic::Packets::FrequencySweep::start, Events::FreqSweep::start{} },
            T_Pair{ Magic::Packets::FrequencySweep::stop_frequency_sweep, Events::FreqSweep::end{} },
        };
    public:
        constexpr Decoder() = default;
        T_Variant find_event(const Magic::Packets::Packet_T &packet) const {
            size_t i = 0;
            for(const auto &j: jump_info) {
                if(j.first == packet) {
                    std::printf("BLE::Decoder::find_event: found something: i: %zu\n", i);
                    return j.second;
                }
                i++;
            }
            std::printf("BLE::Decoder::find_event: found nothing: i: %zu\n", i);
            return Events::disconnect{};
        }
    };

    namespace Server {
        void stop() {//! Call this function to stop BLE 
            std::printf("BLE::Server::stop\n");
            //! Below is the sequence of APIs to be called to disable/deinit NimBLE host and ESP controller:
            printf("\n Stoping BLE and notification task \n");
            if(Trielo::trielo<nimble_port_stop>(Trielo::OkErrCode(0)) == 0) {
                Trielo::trielo<nimble_port_deinit>(Trielo::OkErrCode(ESP_OK));
            }
            Trielo::trielo<esp_bt_controller_disable>(Trielo::OkErrCode(ESP_OK));
        }

        void heartbeat_cb() {
            std::printf("BLE::Server::heartbeat_cb\n");
            char write_buffer[20];
            struct os_mbuf *txom;
            while(heartbeat_running) {
                std::printf("From heartbeat callback\n");
                //const float temperature = AD5933_Tests::ad5933.load()->measure_temperature().value_or(0xFFFF'FFFF);
                const float temperature = 0.0f;
                std::sprintf(write_buffer, "%f °C", temperature);
                std::printf("Heartbeat callback: write_buffer: %s\n", write_buffer);
                txom = ble_hs_mbuf_from_flat(write_buffer, sizeof(write_buffer));
                Trielo::trielo<&ble_gatts_indicate_custom>(Trielo::OkErrCode(0), conn_handle, body_composition_measurement_characteristic_handle, txom);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } 
        }

        void create_heartbeat_task() {
            std::printf("BLE::Server::create_heartbeat_task\n");
            if(heartbeat_running == true) {
                return;
            }
            heartbeat_running = true;
            std::thread(heartbeat_cb).detach();
        }

        static void wakeup_cb() {
            std::printf("BLE::Server::wakeup_cb\n");
            std::this_thread::sleep_for(std::chrono::seconds(10));
            dummy_state_machine.process_event(BLE::Events::wakeup{});
        }

        static int gap_event_cb(struct ble_gap_event *event, void *arg) {
            std::printf("BLE::Server::gap_event_cb\n");
            struct ble_gap_conn_desc desc;
            int rc;
            switch(event->type) {
            case BLE_GAP_EVENT_CONNECT:
                /* A new connection was established or a connection attempt failed. */
                MODLOG_DFLT(INFO, "connection %s; status=%d ",
                            event->connect.status == 0 ? "established" : "failed",
                            event->connect.status);
                if(event->connect.status == 0) {
                    rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
                    assert(rc == 0);
                }
                MODLOG_DFLT(INFO, "\n");

                if(event->connect.status != 0) {
                    /* Connection failed; resume advertising. */
                    dummy_state_machine.process_event(BLE::Events::advertise{});
                }
                conn_handle = event->connect.conn_handle;
                dummy_state_machine.process_event(BLE::Events::connect{});
                break;

            case BLE_GAP_EVENT_DISCONNECT:
                MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
                MODLOG_DFLT(INFO, "\n");

                /* Connection terminated; resume advertising. */
                /* Again commenting out intentionally disabling
                if(heartbeat_running == true) {
                    std::cout << "Deleting heartbeat task\n";
                    heartbeat_running = false;
                    heartbeat_thread.value().join();
                } */ 

                dummy_state_machine.process_event(BLE::Events::disconnect{});
                break;

            case BLE_GAP_EVENT_CONN_UPDATE:
                /* The central has updated the connection parameters. */
                MODLOG_DFLT(INFO, "connection updated; status=%d ",
                            event->conn_update.status);
                rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
                assert(rc == 0);
                MODLOG_DFLT(INFO, "\n");
                break;

            case BLE_GAP_EVENT_ADV_COMPLETE:
                MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                            event->adv_complete.reason);
                /* Commented out disabling heartbeat subscribe event
                if(heartbeat_running == true) {
                    std::cout << "Deleting heartbeat task\n";
                    heartbeat_running = false;
                    heartbeat_thread.value().join();
                } */
                if(event->adv_complete.reason == BLE_HS_ETIMEOUT) {
                    //std::thread([]() {
                        //std::thread(wakeup_cb).detach();
                        //dummy_state_machine.process_event(BLE::Events::sleep{});
                    //}).detach();
                    dummy_state_machine.process_event(BLE::Events::sleep{});
                }
                dummy_state_machine.process_event(BLE::Events::advertise{});
                break;

            case BLE_GAP_EVENT_SUBSCRIBE:
                MODLOG_DFLT(INFO, "subscribe event; cur_notify=%d\n value handle; "
                                    "val_handle=%d\n"
                                    "conn_handle=%d attr_handle=%d "
                                    "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                            event->subscribe.conn_handle,
                            event->subscribe.attr_handle,
                            event->subscribe.reason,
                            event->subscribe.prev_notify,
                            event->subscribe.cur_notify,
                            event->subscribe.cur_notify, body_composition_measurement_characteristic_handle, //!! Client Subscribed to body_composition_measurement_characteristic_handle
                            event->subscribe.prev_indicate,
                            event->subscribe.cur_indicate);

                if(event->subscribe.attr_handle == body_composition_measurement_characteristic_handle) {
                    if(event->subscribe.cur_indicate != 0) {
                        //state_machine.change_to_state(static_cast<const States::bState*>(&States::subscribed));
                        printf("\nSubscribed with body_composition_measurement_characteristic_handle =%d\n", event->subscribe.attr_handle);
                    } else {
                        if(heartbeat_running == true) {
                            //state_machine.change_to_state(static_cast<const States::bState*>(&States::unsubscribe));
                        }
                    }
                }
                break;

            case BLE_GAP_EVENT_MTU:
                MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                            event->mtu.conn_handle,
                            event->mtu.channel_id,
                            event->mtu.value);
                break;
            }
            return 0;
        }

        static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *len) {
            std::printf("BLE::Server::gatt_svr_chr_write\n");
            uint16_t om_len;
            int rc;

            om_len = OS_MBUF_PKTLEN(om);
            if(om_len < min_len || om_len > max_len) {
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }

            rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
            if (rc != 0) {
                return BLE_ATT_ERR_UNLIKELY;
            }

            return 0;
        }

        int body_composition_feature_characteristic_access_cb(
            uint16_t conn_handle,
            uint16_t attr_handle,
            struct ble_gatt_access_ctxt *ctxt,
            void *arg
        ) {
            std::printf("BLE::Server::body_composition_feature_characteristic_access_cb\n");
            int rc;
            struct os_mbuf *txom;
            Magic::Packets::Packet_T received_packet { 0x00 };
            static constexpr Decoder decoder {};
            switch(ctxt->op) {
            case BLE_GATT_ACCESS_OP_WRITE_CHR: //!! In case user accessed this characterstic to write, bellow lines will executed.
                rc = gatt_svr_chr_write(ctxt->om, 1, 700, &characteristic_received_value, NULL); //!! Function "gatt_svr_chr_write" will fire.
                std::printf("Received=%s\n", characteristic_received_value);  // Print the received value
                for(size_t i = 0; i < Magic::Packets::Debug::start.size(); i++) {
                    if(i % 8 == 0) {;
                        std::printf("\n");
                    }
                    std::printf("0x%02X, ", characteristic_received_value[i]);
                }
                std::memcpy(received_packet.data(), reinterpret_cast<uint8_t*>(characteristic_received_value), received_packet.size()); 
                std::memset(characteristic_received_value, 0, sizeof(characteristic_received_value));
                std::visit([](auto &&arg) {
                    dummy_state_machine.process_event(arg);
                }, decoder.find_event(received_packet));
                return rc;
            }
            return BLE_ATT_ERR_UNLIKELY;
        }

        void advertise() {
            std::printf("BLE::Server::advertise\n");
            static constexpr ble_uuid16_t uuids16 {
                .u = BLE_UUID_TYPE_16,
                .value = PROFILE_BODY_COMPOSITION_UUID,
            };

            static constexpr uint8_t name[] { 'n', 'i', 'm', 'b', 'l', 'e' };

            static constexpr ble_hs_adv_fields adv_fields {
                .flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP,
                .uuids16 = &uuids16,
                .num_uuids16 = 1,
                .uuids16_is_complete = 1,
                .uuids32 = nullptr,
                .num_uuids32 = 0,
                .uuids32_is_complete = 0,
                .uuids128 = nullptr,
                .num_uuids128 = 0,
                .uuids128_is_complete = 0,
                .name = name,
                .name_len = 6,
                .name_is_complete = 1,
                .tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO,
                .tx_pwr_lvl_is_present = 1,
                .slave_itvl_range = nullptr,
                .svc_data_uuid16 = nullptr,
                .svc_data_uuid16_len = 0,
                .public_tgt_addr = nullptr,
                .num_public_tgt_addrs = 0,
                .appearance = APPEARANCE_GENERIC_SENSOR_UUID,
                .appearance_is_present = 1,
                .adv_itvl = 0,
                .adv_itvl_is_present = 0,
                .svc_data_uuid32 = nullptr,
                .svc_data_uuid32_len = 0,
                .svc_data_uuid128 = nullptr,
                .svc_data_uuid128_len = 0,
                .uri = nullptr,
                .uri_len = 0,
                .mfg_data = nullptr,
                .mfg_data_len = 0
            };

            Trielo::trielo<&ble_gap_adv_set_fields>(Trielo::OkErrCode(0), &adv_fields);

            static constexpr ble_gap_adv_params adv_params {
                .conn_mode = BLE_GAP_CONN_MODE_UND,
                .disc_mode = BLE_GAP_DISC_MODE_GEN,
                .itvl_min = 0,
                .itvl_max = 0,
                .channel_map = 0,
                .filter_policy = 0,
                .high_duty_cycle = 0,
            };

            //static constexpr int32_t advertising_timeout_ms = BL_HS_FOREVER;
            static constexpr int32_t advertising_timeout_ms = 60'000;
            Trielo::trielo<&ble_gap_adv_start>(
                Trielo::OkErrCode(0),
                own_addr_type,
                nullptr,
                advertising_timeout_ms,
                &adv_params,
                &gap_event_cb,
                nullptr
            );
        }

        void reset_cb(int reason) {
            std::printf("BLE::Server::reset_cb: reason: %d\n", reason);
        }

        static void sync_cb() {
            std::printf("BLE::Server::sync_cb\n");
            /* Generate a non-resolvable private address. */
            Trielo::trielo<&ble_hs_util_ensure_addr>(Trielo::OkErrCode(0), 0);
            Trielo::trielo<&ble_hs_id_infer_auto>(Trielo::OkErrCode(0), 0, &own_addr_type);
        }

        void gatt_register_cb(ble_gatt_register_ctxt *ctxt, void *arg) {
            std::printf("BLE::Server::gatt_register_cb\n");
            char buf[BLE_UUID_STR_LEN];

            switch(ctxt->op) {
                case BLE_GATT_REGISTER_OP_SVC:
                    MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                                ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                                ctxt->svc.handle);
                    break;

                case BLE_GATT_REGISTER_OP_CHR:
                    MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                                        "def_handle=%d val_handle=%d\n",
                                ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                                ctxt->chr.def_handle,
                                ctxt->chr.val_handle);
                    break;

                case BLE_GATT_REGISTER_OP_DSC:
                    MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                                ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                                ctxt->dsc.handle);
                    break;

                default:
                    assert(0);
                    break;
            }
        }

        int body_composition_measurement_characteristic_access_cb(
            uint16_t conn_handle,
            uint16_t attr_handle,
            struct ble_gatt_access_ctxt *ctxt,
            void *arg
        ) {
            std::printf("BLE::Server::body_composition_measurement_characteristic_access_cb\n");
            /* Is supposed to be empty because no write or read access only notify and indicate */
            return 0;
        }

        void task_cb(void *param) {
            std::printf("BLE::Server::task_cb\n");
            Trielo::trielo<nimble_port_run>();
            Trielo::trielo<nimble_port_freertos_deinit>();
        }

        void inject(const i2c_master_dev_handle_t handle) {
            std::printf("BLE::Server::inject\n");
            dummy_driver.device_handle = handle;
            dummy_state_machine.process_event(Events::turn_on{});
        }

        void run() {
            //esp_nimble_hci_and_controller_init();
            std::printf("BLE::Server::run\n");
            esp_err_t ret = Trielo::trielo<nvs_flash_init>(Trielo::OkErrCode(ESP_OK));
            if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                Trielo::trielo<nvs_flash_erase>(Trielo::OkErrCode(ESP_OK));
                Trielo::trielo<nvs_flash_init>(Trielo::OkErrCode(ESP_OK));
            }
            
            Trielo::trielo<nimble_port_init>(Trielo::OkErrCode(ESP_OK));

            ble_hs_cfg.reset_cb = &reset_cb;
            ble_hs_cfg.sync_cb = &sync_cb;
            ble_hs_cfg.gatts_register_cb = &gatt_register_cb;
            ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
            ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
            ble_hs_cfg.sm_sc = 0;

            Trielo::trielo<ble_svc_gap_init>();
            Trielo::trielo<ble_svc_gatt_init>();

            static constexpr ble_uuid_any_t body_composition_feature_characteristic_uuid {
                .u16 = {
                    .u = BLE_UUID_TYPE_16,
                    .value = CHARACTERISTIC_BODY_COMPOSITION_FEATURE_UUID
                },
            };

            static constexpr ble_uuid_any_t body_composition_measurement_characteristic_uuid {
                .u16 = {
                    .u = BLE_UUID_TYPE_16,
                    .value = CHARACTERISTIC_BODY_COMPOSITION_MEASUREMENT_UUID
                }
            };

            // Initialize your characteristic definition.
            static constexpr ble_gatt_chr_def body_composition_feature_characteristic {
                .uuid = &body_composition_feature_characteristic_uuid.u,
                .access_cb = body_composition_feature_characteristic_access_cb,
                .arg = nullptr,  // Replace with your actual argument.
                .descriptors = nullptr,
                .flags = BLE_GATT_CHR_F_WRITE,  // Replace with your flags.
                .min_key_size = 0,  // Replace with your minimum key size.
                .val_handle = &body_composition_feature_characteristic_handle,  // The value handle will be filled in at registration time.
            };

            // Initialize your characteristic definition.
            static constexpr ble_gatt_chr_def body_composition_measurement_characteristic {
                .uuid = &body_composition_measurement_characteristic_uuid.u,
                .access_cb = body_composition_measurement_characteristic_access_cb,
                .arg = nullptr,  // Replace with your actual argument.
                .descriptors = nullptr,
                .flags = BLE_GATT_CHR_F_INDICATE | BLE_GATT_CHR_F_NOTIFY,
                .min_key_size = 0,  // Replace with your minimum key size.
                .val_handle = &body_composition_measurement_characteristic_handle,  // The value handle will be filled in at registration time.
            };

            static constexpr ble_uuid_any_t body_composition_service_uuid {
                .u16 = {
                    .u = BLE_UUID_TYPE_16,
                    .value = 0x181B
                }
            };

            static constexpr ble_gatt_chr_def body_composition_service_characteristics[] { 
                body_composition_feature_characteristic,
                body_composition_measurement_characteristic,
                { nullptr, nullptr, nullptr, nullptr, 0, 0, nullptr },
            };

            static constexpr ble_gatt_svc_def body_composition_service {
                .type = BLE_GATT_SVC_TYPE_PRIMARY,  // Or BLE_GATT_SVC_TYPE_SECONDARY.
                .uuid = &body_composition_service_uuid.u,
                .includes = nullptr,  // If there are no included services.
                .characteristics = body_composition_service_characteristics,
            };

            static constexpr ble_gatt_svc_def gatt_services[] {
                body_composition_service,
                { 0, nullptr, nullptr, nullptr },
            };

            Trielo::trielo<ble_gatts_count_cfg>(Trielo::OkErrCode(0), gatt_services);
            Trielo::trielo<&ble_gatts_add_svcs>(Trielo::OkErrCode(0), gatt_services);
            Trielo::trielo<ble_att_set_preferred_mtu>(Trielo::OkErrCode(0), 23);
            Trielo::trielo<nimble_port_freertos_init>(task_cb);
            dummy_state_machine.process_event(Events::advertise{});
        }
    }

    namespace Server {
        bool notify(const Magic::Packets::Packet_T &message) {
            struct os_mbuf *txom = ble_hs_mbuf_from_flat(message.data(), message.size());
            if(txom == nullptr) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ");
                std::cout << "BLE::Servernotify: failed to ble_hs_mbuf_from_flat\n";
                return false;
            }

            if(ble_gatts_notify_custom(conn_handle, body_composition_measurement_characteristic_handle, txom) == 0) {
                return true;
            } else {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ");
                std::cout << "BLE::Server::notify: failed to ble_gatts_notify_custom\n";
                return false;
            }
        }
    }
}
