#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <memory>

#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "modlog/modlog.h"

#include "ble.hpp"
#include "trielo/trielo.hpp"
#include "ad5933.hpp"

namespace NimBLE {
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
    static uint16_t conn_handle;
    static uint16_t body_composition_feature_characteristic_handle;
    static uint16_t body_composition_measurement_characteristic_handle;
    static uint8_t notify_state;

    static char characteristic_received_value[500];
    
    static void advertise();

    static void set_addr() {
        Trielo::trielo<&ble_hs_util_ensure_addr>(Trielo::OkErrCode(0),
            0
        );

	    Trielo::trielo<&ble_hs_id_infer_auto>(Trielo::OkErrCode(0), 
		    0,
		    &own_addr_type
	    );
    }

    void stopBLE() {//! Call this function to stop BLE 
        //! Below is the sequence of APIs to be called to disable/deinit NimBLE host and ESP controller:
        printf("\n Stoping BLE and notification task \n");
        // vTaskDelete(xHandle);
        int ret = nimble_port_stop();
        if(ret == 0) {
            nimble_port_deinit();
            /*
            ret = esp_nimble_hci_and_controller_deinit();
            if(ret != ESP_OK) {
                ESP_LOGE(tag, "esp_nimble_hci_and_controller_deinit() failed with error: %d", ret);
            }
            */
        }
    }

    void heartbeat_cb(void *arg) {
        char write_buffer[20];
        struct os_mbuf *txom;
        while(1) {
            const float temperature = AD5933_Tests::ad5933.load()->measure_temperature().value_or(0xFFFF'FFFF);
            std::sprintf(write_buffer, "%f °C", temperature);
            txom = ble_hs_mbuf_from_flat(write_buffer, sizeof(write_buffer));
            Trielo::trielo<&ble_gatts_indicate_custom>(Trielo::OkErrCode(0), conn_handle, body_composition_measurement_characteristic_handle, txom);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } 
    }

    static TaskHandle_t heartbeat_task_handle = nullptr;

	void create_heartbeat_task() {
		const uint32_t STACK_SIZE = 2*1024;
		const uint32_t priority = 1;

		/* Create the task, storing the handle. */
		xTaskCreate(
			heartbeat_cb, /* Function that implements the task. */
			"heartbeat_task", /* Text name for the task. */
			STACK_SIZE,       /* Stack size in words, not bytes. */
			nullptr,        /* Parameter passed into the task. */
			priority, /* Priority at which the task is created. */
            &heartbeat_task_handle
		);      
	}

    static int gap_event_cb(struct ble_gap_event *event, void *arg) {
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
				//bleprph_print_conn_desc(&desc);
			}
			MODLOG_DFLT(INFO, "\n");

			if(event->connect.status != 0) {
				/* Connection failed; resume advertising. */
				advertise();
			}
			conn_handle = event->connect.conn_handle;
            break;

		case BLE_GAP_EVENT_DISCONNECT:
			MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
			//bleprph_print_conn_desc(&event->disconnect.conn);
			MODLOG_DFLT(INFO, "\n");

			/* Connection terminated; resume advertising. */
            if(notify_state != 0) {
                vTaskDelete(heartbeat_task_handle);
            }
			advertise();
            break;

		case BLE_GAP_EVENT_CONN_UPDATE:
			/* The central has updated the connection parameters. */
			MODLOG_DFLT(INFO, "connection updated; status=%d ",
						event->conn_update.status);
			rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
			assert(rc == 0);
			//bleprph_print_conn_desc(&desc);
			MODLOG_DFLT(INFO, "\n");
            break;

		case BLE_GAP_EVENT_ADV_COMPLETE:
			MODLOG_DFLT(INFO, "advertise complete; reason=%d",
						event->adv_complete.reason);
            if(notify_state != 0) {
                vTaskDelete(heartbeat_task_handle);
            }
			advertise();
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

			if (event->subscribe.attr_handle == body_composition_measurement_characteristic_handle) {
				printf("\nSubscribed with body_composition_measurement_characteristic_handle =%d\n", event->subscribe.attr_handle);
				notify_state = event->subscribe.cur_indicate; //!! As the client is now subscribed to notifications, the value is set to 1
				printf("notify_state=%u\n", notify_state);
                if(notify_state != 0) {
                    create_heartbeat_task();
                } else {
                    vTaskDelete(heartbeat_task_handle);
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
        int rc;
        const char hello_world[] = "hello_world!";
        switch(ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR: //!! In case user accessed this characterstic to read its value, bellow lines will execute
            rc = os_mbuf_append(ctxt->om, &hello_world ,
                                sizeof(hello_world));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        /*
        case BLE_GATT_ACCESS_OP_WRITE_CHR: //!! In case user accessed this characterstic to write, bellow lines will executed.
            rc = gatt_svr_chr_write(ctxt->om, 1, 700, &characteristic_received_value, NULL); //!! Function "gatt_svr_chr_write" will fire.
            printf("Received=%s\n", characteristic_received_value);  // Print the received value
            //! Use received value in you code. For example
            char stp[]="stop";
            int x=strcmp(characteristic_received_value,stp);
            if(x==0){
                stopBLE();
            }
            return rc;
        }
        */
        }
        return BLE_ATT_ERR_UNLIKELY;
    }

    static void advertise() {
        static constinit const ble_gap_adv_params adv_params {
            .conn_mode = BLE_GAP_CONN_MODE_UND,
            .disc_mode = BLE_GAP_DISC_MODE_GEN,
            .itvl_min = 0,
            .itvl_max = 0,
            .channel_map = 0,
            .filter_policy = 0,
            .high_duty_cycle = 0,
        };

        static constinit const ble_uuid16_t uuids16 {
            .u = BLE_UUID_TYPE_16,
            .value = PROFILE_BODY_COMPOSITION_UUID,
        };

        static constinit const ble_hs_adv_fields adv_fields = {
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
            .name = reinterpret_cast<const uint8_t*>("nimble"),
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

        Trielo::trielo<&ble_gap_adv_set_fields>(
            Trielo::OkErrCode(0),
            &adv_fields
        );

        Trielo::trielo<&ble_gap_adv_start>(
            Trielo::OkErrCode(0),
            own_addr_type,
            nullptr,
            BLE_HS_FOREVER,
            &adv_params,
            &gap_event_cb,
            nullptr
        );
    }

    void reset_cb(int reason) {
        std::cout << "BLE resetting: '" << reason << "'\n";
    }

    static void sync_cb() {
        /* Generate a non-resolvable private address. */
        set_addr();

        /* Advertise indefinitely. */
        advertise();
    }

    void gatt_register_cb(ble_gatt_register_ctxt *ctxt, void *arg) {
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

    void task_cb(void *param) {
	    //ESP_LOGI(tag, "BLE Host Task Started");
	    /* This function will return only when nimble_port_stop() is executed */
	    nimble_port_run();
	    nimble_port_freertos_deinit();
    }

    int body_composition_measurement_characteristic_access_cb(
        uint16_t conn_handle,
        uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt,
        void *arg
    ) {
        return 0;
    }
}

namespace NimBLE {
    void run() {
        esp_err_t ret = nvs_flash_init();
        if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
        
        Trielo::trielo<&nimble_port_init>(Trielo::OkErrCode(ESP_OK));

        ble_hs_cfg.reset_cb = &reset_cb;
        ble_hs_cfg.sync_cb = &sync_cb;
        ble_hs_cfg.gatts_register_cb = &gatt_register_cb;
	    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
	    ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
	    ble_hs_cfg.sm_sc = 0;

        ble_svc_gap_init();
        ble_svc_gatt_init();

        static constinit const ble_uuid16_t body_composition_feature_characteristic_uuid {
            .u = BLE_UUID_TYPE_16,
            .value = CHARACTERISTIC_BODY_COMPOSITION_FEATURE_UUID
        };

        static constinit const ble_uuid16_t body_composition_measurement_characteristic_uuid {
            .u = BLE_UUID_TYPE_16,
            .value = CHARACTERISTIC_BODY_COMPOSITION_MEASUREMENT_UUID
        };

        // Initialize your characteristic definition.
        static constinit const ble_gatt_chr_def body_composition_feature_characteristic {
            .uuid = reinterpret_cast<const ble_uuid_t*>(&body_composition_feature_characteristic_uuid),
            .access_cb = body_composition_feature_characteristic_access_cb,
            .arg = nullptr,  // Replace with your actual argument.
            .descriptors = nullptr,
            .flags = BLE_GATT_CHR_F_READ,  // Replace with your flags.
            .min_key_size = 0,  // Replace with your minimum key size.
            .val_handle = &body_composition_feature_characteristic_handle,  // The value handle will be filled in at registration time.
        };

        // Initialize your characteristic definition.
        static constinit const ble_gatt_chr_def body_composition_measurement_characteristic {
            .uuid = reinterpret_cast<const ble_uuid_t*>(&body_composition_measurement_characteristic_uuid),
            .access_cb = body_composition_measurement_characteristic_access_cb,
            .arg = nullptr,  // Replace with your actual argument.
            .descriptors = nullptr,
            .flags = BLE_GATT_CHR_F_INDICATE,
//                | (0x01 << 1) /* Time stamp bit is set */
//                | (0x01 << 2) /* UserID bit is set*/
//                | (0x01 << 9),/* Impedance bit is set */
            .min_key_size = 0,  // Replace with your minimum key size.
            .val_handle = &body_composition_measurement_characteristic_handle,  // The value handle will be filled in at registration time.
        };

        /*
        std::printf("body_composition_feature_characteristic:");
        for(size_t i = sizeof(body_composition_feature_characteristic) - 1; i > 0; i--) {
            std::printf("%02X, ", reinterpret_cast<const uint8_t*>(&body_composition_feature_characteristic)[i]);
            if(i % 8 == 0) {
                std::printf("\n");
            }
        }
        */

        static constinit const ble_uuid16_t body_composition_service_uuid {
            .u = BLE_UUID_TYPE_16,
            .value = 0x181B
        };

        static constinit const ble_gatt_chr_def body_composition_service_characteristics[] { 
            body_composition_feature_characteristic,
            body_composition_measurement_characteristic,
            {0}
        };

        static constinit const ble_gatt_svc_def body_composition_service {
            .type = BLE_GATT_SVC_TYPE_PRIMARY,  // Or BLE_GATT_SVC_TYPE_SECONDARY.
            .uuid = reinterpret_cast<const ble_uuid_t*>(&body_composition_service_uuid),
            .includes = nullptr,  // If there are no included services.
            .characteristics = body_composition_service_characteristics,
        };

        static constinit const ble_gatt_svc_def gatt_services[] {
            body_composition_service,
            {0}
        };

        Trielo::trielo<&ble_gatts_count_cfg>(
            Trielo::OkErrCode(0),
            gatt_services
        );

        Trielo::trielo<&ble_gatts_add_svcs>(
            Trielo::OkErrCode(0),
            gatt_services
        );
       
        ble_svc_gap_device_name_set("nimble-ble"); //!! Set the name of this device
        nimble_port_freertos_init(&task_cb);
    }
}
