#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <memory>
#include <atomic>
#include <array>
#include <cstdint>
#include <variant>
#include <unordered_map>
#include <functional>
#include <utility>
#include <ctime>

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
#include "magic/packets/incoming.hpp"
#include "util.hpp"

#include "ad5933/driver/driver.hpp"
#include "ad5933/extension/extension.hpp"
#include "ble/server/server.hpp"

namespace BLE {
    namespace Server {
        static uint8_t own_addr_type;
        static uint16_t conn_handle = 0;
        namespace CharHandles {
            static uint16_t body_composition_feature;
            static uint16_t body_composition_measurement;
            static uint16_t time_update_control_point;
            static uint16_t hid_control_information;
        }
    }

    namespace Server {
        std::shared_ptr<Server::Sender> sender { std::make_shared<Server::Sender>() };
        AD5933::Driver dummy_driver {};
        AD5933::Extension dummy_extension { dummy_driver };
        std::atomic<bool> processing = false;
        my_logger logger {};
        StopSources stop_sources {};
        T_StateMachine dummy_state_machine { logger, sender, dummy_extension, processing, stop_sources };
    }

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
                    advertise(); 
                }
                conn_handle = event->connect.conn_handle;
                dummy_state_machine.process_event(BLE::Events::connect{});
                break;

            case BLE_GAP_EVENT_DISCONNECT:
                MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
                MODLOG_DFLT(INFO, "\n");
                dummy_state_machine.process_event(BLE::Events::disconnect{});
                break;
            case BLE_GAP_EVENT_CONN_UPDATE:
                MODLOG_DFLT(INFO, "connection updated; status=%d ",
                            event->conn_update.status);
                rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
                assert(rc == 0);
                MODLOG_DFLT(INFO, "\n");
                break;

            case BLE_GAP_EVENT_ADV_COMPLETE:
                MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                            event->adv_complete.reason);
                if(event->adv_complete.reason == BLE_HS_ETIMEOUT) {
                    dummy_state_machine.process_event(BLE::Events::sleep{});
                }
                break;
            case BLE_GAP_EVENT_SUBSCRIBE:
                MODLOG_DFLT(INFO,   "subscribe event; " "conn_handle=%d; " "attr_handle=%d;\n"
                                    "reason=%d; " "prev_notify=%d; " "cur_notify=%d;\n"
                                    "body_composition_measurement_characteristic_handle=%d;\n"
                                    "prev_indicate=%d; " "cur_indicate=%d;\n",
                            event->subscribe.conn_handle,
                            event->subscribe.attr_handle,
                            event->subscribe.reason,
                            event->subscribe.prev_notify,
                            event->subscribe.cur_notify,
                            CharHandles::body_composition_measurement,
                            event->subscribe.prev_indicate,
                            event->subscribe.cur_indicate);
                if(event->subscribe.cur_notify == 0 && event->subscribe.prev_notify == 1) {
                    dummy_state_machine.process_event(Events::disconnect{});
                }
                break;

            case BLE_GAP_EVENT_MTU:
                MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                            event->mtu.conn_handle,
                            event->mtu.channel_id,
                            event->mtu.value);
                break;
            case BLE_GAP_EVENT_NOTIFY_TX:
                break;
            case BLE_GAP_EVENT_NOTIFY_RX:
                break;
            default:
    		    fmt::print(fmt::fg(fmt::color::red), "WE HERE: PROBABLY UNKNOWN UNHANDLED GAP EVENT: {}\n", event->type);
                break;
            }
            return 0;
        }

        static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *out_copy_len) {
            std::printf("BLE::Server::gatt_svr_chr_write\n");
            const uint16_t om_len = OS_MBUF_PKTLEN(om);
            if(om_len < min_len || om_len > max_len) {
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }

            const int rc = ble_hs_mbuf_to_flat(om, dst, max_len, out_copy_len);
            if (rc != 0) {
                return BLE_ATT_ERR_UNLIKELY;
            }

            return 0;
        }
        
        void print_received_packet(const Magic::T_MaxPacket& packet) {
            std::printf("BLE::Server::print_received_packet:");
            std::for_each(packet.begin(), packet.end(), [index = 0](const auto& e) mutable {
                if(index % 8 == 0) {;
                    std::printf("\n\t");
                }
                std::printf("0x%02X, ", e);
                index++;
            });
            std::printf("\n");
        }

        int body_composition_feature_characteristic_access_cb(
            uint16_t conn_handle,
            uint16_t attr_handle,
            struct ble_gatt_access_ctxt *ctxt,
            void *arg
        ) {
            std::printf("BLE::Server::body_composition_feature_characteristic_access_cb\n");
            switch(ctxt->op) {
            case BLE_GATT_ACCESS_OP_WRITE_CHR: //!! In case user accessed this characterstic to write, bellow lines will executed.
                Magic::T_MaxPacket tmp_characteristic_received_value { 0 };
                const int rc = gatt_svr_chr_write(ctxt->om, 1, tmp_characteristic_received_value.size(), tmp_characteristic_received_value.data(), NULL); //!! Function "gatt_svr_chr_write" will fire.
                print_received_packet(tmp_characteristic_received_value);
                const Magic::InComingPacket<Magic::Events::Commands::Variant, Magic::Events::Commands::Map> incoming_packet { tmp_characteristic_received_value };
                auto event_variant { incoming_packet.to_event_variant() };
                if(event_variant.has_value()) {
                    std::visit([](auto &&arg) {
                        dummy_state_machine.process_event(arg);
                    }, incoming_packet.to_event_variant().value());
                }
                return rc;
            }
            return BLE_ATT_ERR_UNLIKELY;
        }

        int time_update_control_point_characteristic_access_cb(
            uint16_t conn_handle,
            uint16_t attr_handle,
            struct ble_gatt_access_ctxt *ctxt,
            void *arg
        ) {
            std::printf("BLE::Server::time_update_control_point_characteristic_access_cb\n");
            switch(ctxt->op) {
            case BLE_GATT_ACCESS_OP_WRITE_CHR: //!! In case user accessed this characterstic to write, bellow lines will executed.
                Magic::T_MaxPacket tmp_characteristic_received_value { 0 };
                const int rc = gatt_svr_chr_write(ctxt->om, 1, tmp_characteristic_received_value.size(), tmp_characteristic_received_value.data(), NULL); //!! Function "gatt_svr_chr_write" will fire.
                print_received_packet(tmp_characteristic_received_value);
                Magic::InComingPacket<Magic::Events::Commands::Variant, Magic::Events::Commands::Map> incoming_packet { tmp_characteristic_received_value };
                auto event_variant { incoming_packet.to_event_variant() };
                if(event_variant.has_value()) {
                    std::visit([](auto &&arg) {
                        using T_Decay = std::decay_t<decltype(arg)>;
                       if constexpr (std::is_same_v<T_Decay, Magic::Events::Commands::Time::UpdateTimeval>) {
                            settimeofday(&arg.tv, nullptr);
                            std::printf("BLE_Server::time_update_control_point_characteristic_access_cb: Updated timeval\n");
                            Util::print_current_time();
                        } else if constexpr (std::is_same_v<T_Decay, Magic::Events::Commands::Time::UpdateTimezone>) {
                            settimeofday(nullptr, &arg.tz);
                            std::printf("BLE_Server::time_update_control_point_characteristic_access_cb: Updated timezone\n");
                            Util::print_current_time();
                        }
                    }, incoming_packet.to_event_variant().value());
                }
                return rc;
            }
            return BLE_ATT_ERR_UNLIKELY;
        }

        int hid_control_information_characteristic_access_cb(
            uint16_t conn_handle,
            uint16_t attr_handle,
            struct ble_gatt_access_ctxt *ctxt,
            void *arg
        ) {
            std::printf("BLE::Server::hid_control_information_characteristic_access_cb\n");
            /* Is supposed to be empty because no write or read access only notify and indicate */
            return 0;
        }

        void advertise() {
            static constexpr ble_uuid16_t profile_body_composition_uuid {
                .u = BLE_UUID_TYPE_16,
                .value = 0x1014,
            };

            static constexpr uint8_t name[] { 'n', 'i', 'm', 'b', 'l', 'e' };
            static constexpr uint8_t mfg_data[] { 'n', 'i', 'm', 'b', 'l', 'e' };
            static constexpr uint16_t appearance_generic_sensor_uuid = 0x0540;

            static constexpr ble_hs_adv_fields adv_fields {
                .flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP,
                .uuids16 = &profile_body_composition_uuid,
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
                .appearance = appearance_generic_sensor_uuid,
                .appearance_is_present = 1,
                .adv_itvl = 0,
                .adv_itvl_is_present = 0,
                .svc_data_uuid32 = nullptr,
                .svc_data_uuid32_len = 0,
                .svc_data_uuid128 = nullptr,
                .svc_data_uuid128_len = 0,
                .uri = nullptr,
                .uri_len = 0,
                .mfg_data = mfg_data,
                .mfg_data_len = sizeof(mfg_data)
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

            //static constexpr int32_t advertising_timeout_ms = BLE_HS_FOREVER;
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
            Trielo::trielo<&ble_hs_util_ensure_addr>(Trielo::OkErrCode(0), 0);
            Trielo::trielo<&ble_hs_id_infer_auto>(Trielo::OkErrCode(0), 0, &own_addr_type);
            dummy_state_machine.process_event(Events::advertise{});
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
            dummy_driver.device_handle = handle;
            dummy_state_machine.process_event(Events::turn_on{});
        }

        void run() {
            const esp_err_t ret = Trielo::trielo<nvs_flash_init>(Trielo::OkErrCode(ESP_OK));
            if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                Trielo::trielo<nvs_flash_erase>(Trielo::OkErrCode(ESP_OK));
                Trielo::trielo<nvs_flash_init>(Trielo::OkErrCode(ESP_OK));
            }
            
            Trielo::trielo<nimble_port_init>(Trielo::OkErrCode(ESP_OK));

            ble_hs_cfg.reset_cb = &reset_cb;
            ble_hs_cfg.sync_cb = &sync_cb;
            ble_hs_cfg.gatts_register_cb = &gatt_register_cb;
            ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
            ble_hs_cfg.sm_io_cap = 3;
            ble_hs_cfg.sm_sc = 0;

            Trielo::trielo<ble_svc_gap_init>();
            Trielo::trielo<ble_svc_gatt_init>();

            static constexpr ble_uuid_any_t body_composition_feature_characteristic_uuid {
                .u16 = {
                    .u = BLE_UUID_TYPE_16,
                    .value = 0x2A9B,
                },
            };

            static constexpr ble_uuid_any_t body_composition_measurement_characteristic_uuid {
                .u16 = {
                    .u = BLE_UUID_TYPE_16,
                    .value = 0x2A9C,
                }
            };

            static constexpr ble_uuid_any_t time_update_control_point_uuid {
                .u16 = {
                    .u = BLE_UUID_TYPE_16,
                    .value = 0x2A16
                }
            };

            static constexpr ble_uuid_any_t hid_control_information_uuid {
                .u16 = {
                    .u = BLE_UUID_TYPE_16,
                    .value = 0x2A4A
                }
            };

            static constexpr ble_gatt_chr_def body_composition_feature_characteristic {
                .uuid = &body_composition_feature_characteristic_uuid.u,
                .access_cb = body_composition_feature_characteristic_access_cb,
                .arg = nullptr,
                .descriptors = nullptr,
                .flags = BLE_GATT_CHR_F_WRITE,
                .min_key_size = 0,
                .val_handle = &CharHandles::body_composition_feature,
            };

            static constexpr ble_gatt_chr_def body_composition_measurement_characteristic {
                .uuid = &body_composition_measurement_characteristic_uuid.u,
                .access_cb = body_composition_measurement_characteristic_access_cb,
                .arg = nullptr,
                .descriptors = nullptr,
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .min_key_size = 0,
                .val_handle = &CharHandles::body_composition_measurement,
            };

            static constexpr ble_gatt_chr_def time_update_control_point_characteristic {
                .uuid = &time_update_control_point_uuid.u,
                .access_cb = time_update_control_point_characteristic_access_cb,
                .arg = nullptr,
                .descriptors = nullptr,
                .flags = BLE_GATT_CHR_F_WRITE,
                .min_key_size = 0,
                .val_handle = &CharHandles::time_update_control_point,
            };

            static constexpr ble_gatt_chr_def hid_control_information_characteristic {
                .uuid = &hid_control_information_uuid.u,
                .access_cb = hid_control_information_characteristic_access_cb,
                .arg = nullptr,
                .descriptors = nullptr,
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .min_key_size = 0,
                .val_handle = &CharHandles::hid_control_information,
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
                time_update_control_point_characteristic,
                hid_control_information_characteristic,
                { nullptr, nullptr, nullptr, nullptr, 0, 0, nullptr },
            };

            static constexpr ble_gatt_svc_def body_composition_service {
                .type = BLE_GATT_SVC_TYPE_PRIMARY,
                .uuid = &body_composition_service_uuid.u,
                .includes = nullptr,
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
        }
    }

    namespace Server {
        bool notify_hid_information(const std::span<uint8_t, std::dynamic_extent>& message) {
            assert(message.size() <= Magic::MTU);
            struct os_mbuf *txom = ble_hs_mbuf_from_flat(message.data(), message.size());
            if(txom == nullptr) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ");
                std::cout << "BLE::Server::notify_hid_information: failed to ble_hs_mbuf_from_flat\n";
                return false;
            }
            const int ret = ble_gatts_notify_custom(conn_handle, CharHandles::hid_control_information, txom);
            if(ret == 0) {
                return true;
            } else {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ");
                std::cout << "BLE::Server::notify_hid_information: failed to ble_gatts_notify_custom: " << ret << std::endl;
                return false;
            }
        }

        bool notify_body_composition_measurement(const std::span<uint8_t, std::dynamic_extent>& message) {
            assert(message.size() <= Magic::MTU);
            struct os_mbuf *txom = ble_hs_mbuf_from_flat(message.data(), message.size());
            if(txom == nullptr) {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ");
                std::cout << "BLE::Server::notify_body_composition_measurement: failed to ble_hs_mbuf_from_flat\n";
                return false;
            }
            const int ret = ble_gatts_notify_custom(conn_handle, CharHandles::body_composition_measurement, txom);
            if(ret == 0) {
                return true;
            } else {
    		    fmt::print(fmt::fg(fmt::color::red), "ERROR: ");
                std::cout << "BLE::Server::notify_body_composition_measurement: failed to ble_gatts_notify_custom: " << ret << std::endl;
                return false;
            }
        }
    }
}
