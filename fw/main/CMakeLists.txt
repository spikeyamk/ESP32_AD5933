idf_component_register(SRCS
   "main.cpp"
   "lib/sd_card.cpp"
   "lib/default.cpp"
   "lib/auto_save_no_ble.cpp"
   "lib/ble_server/server.cpp"
   "lib/ble_server/state_machine.cpp"
   INCLUDE_DIRS "include"
   REQUIRES esp_littlefs trielo_wrapper driver sml_wrapper my_util sdmmc vfs ad5933_driver ad5933_extension ad5933_calibration ad5933_measurement nvs_flash bt magic
)