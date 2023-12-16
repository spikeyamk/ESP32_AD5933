if(ESP_PLATFORM)
    idf_component_register(SRCS
        "lib/server.cpp"
        INCLUDE_DIRS "include"
        REQUIRES "nvs_flash" "bt" "trielo_wrapper" "magic"
    )
endif()