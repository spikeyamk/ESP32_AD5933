idf_component_register(SRCS
		       "main.cpp"
		       "lib/util.cpp"
		       "lib/i2c/bus.cpp"
		       "lib/ad5933/driver/driver.cpp"
		       "lib/ad5933/driver/tests.cpp"
		       "lib/ble/ble_server.cpp"
		       #"ble/state_machine/ble.cpp"
		       #"lib/ble/state_machine/state_machine.cpp"
		       #"ble/state_machine/states.cpp"
		       #"ble_state_machine/magic_packets.cpp"
		       #"ble_state_machine/packet_types.cpp"
		       "lib/sd_card.cpp"
                       INCLUDE_DIRS "include")
