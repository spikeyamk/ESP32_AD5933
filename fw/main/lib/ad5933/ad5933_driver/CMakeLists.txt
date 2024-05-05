if(ESP_PLATFORM)
    set(COMPONENT_NAME "ad5933_driver")
    idf_component_register(SRCS
        lib/driver.cpp
        lib/tests.cpp
        INCLUDE_DIRS "include"
        REQUIRES i2c ad5933_reg_addrs ad5933_masks ad5933_config
    )
endif()
