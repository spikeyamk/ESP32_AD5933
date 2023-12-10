
idf_component_register(SRCS 
					   "main.cpp"
					   "util.cpp"
					   "ad5933.cpp"
					   "ble_state_machine/ble.cpp"
					   "ble_state_machine/ble_state_machine.cpp"
					   "ble_state_machine/ble_states.cpp"
					   "ble_state_machine/magic_packets.cpp"
					   "ble_state_machine/packet_types.cpp"
					   "sd_card.cpp"
                       INCLUDE_DIRS ".")

# Publicly link `foo` to `main` component
#target_link_libraries(main PUBLIC sml)
