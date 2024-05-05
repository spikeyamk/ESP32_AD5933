if(ESP_PLATFORM)
    idf_component_register(SRCS
        lib/extension.cpp
        INCLUDE_DIRS "include"
        REQUIRES ad5933_driver
    )
endif()