if(ESP_PLATFORM)
    idf_component_register(SRCS
        "lib/state_machine.cpp"
        "lib/server.cpp"
        #"lib/sender.cpp"
        INCLUDE_DIRS "include"
        REQUIRES "nvs_flash" "bt" "trielo_wrapper" "magic" "sml_wrapper" "extension" "masks" "my_util"
    )
endif()