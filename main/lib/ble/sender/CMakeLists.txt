if(ESP_PLATFORM)
    idf_component_register(SRCS
        "lib/sender.cpp"
        INCLUDE_DIRS "include"
        REQUIRES "server" "magic"
    )
endif()