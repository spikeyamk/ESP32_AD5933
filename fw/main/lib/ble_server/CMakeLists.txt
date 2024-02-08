if(ESP_PLATFORM)
    idf_component_register(SRCS
        "lib/state_machine.cpp"
        "lib/server.cpp"
        INCLUDE_DIRS "include" "../../include"
        REQUIRES "nvs_flash" "bt" "trielo_wrapper" "magic" "sml_wrapper" "extension" "masks" "my_util" "vfs" "spi_flash" "fatfs"
    )
endif()