if(ESP_PLATFORM)
    idf_component_register(
        SRCS
        lib/util.cpp
        INCLUDE_DIRS "include"
        REQUIRES driver trielo_wrapper ad5933_driver
    )
endif()
