if(ESP_PLATFORM)
    idf_component_register(
        SRCS
        lib/bus.cpp
        INCLUDE_DIRS "include"
        REQUIRES driver spi_flash esp_common
    )
endif()