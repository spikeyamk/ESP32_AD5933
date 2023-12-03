#include <iostream>
#include <utility>

#include "ble_states.hpp"
#include "magic_packets.hpp"

class StateFunctions {
public:
	static void off() {
		std::cout << "off" << std::endl; std::cout << " UNIMPLEMENTED" << std::endl;
	}

	static void on() {
		std::cout << "on" << " UNIMPLEMENTED" << std::endl;
	}

	static void advertise() {
		std::cout << "advertise" << " UNIMPLEMENTED" << std::endl;
	}

	static void advertising() {
		std::cout << "advertising" << " UNIMPLEMENTED" << std::endl;
	}

	static void stop_advertising() {
		std::cout << "stop_advertising" << " UNIMPLEMENTED" << std::endl;
	}

	static void connect() {
		std::cout << "connect" << " UNIMPLEMENTED" << std::endl;
	}

	static void connected() {
		std::cout << "connected" << " UNIMPLEMENTED" << std::endl;
	}

	static void disconnect() {
		std::cout << "disconnect" << " UNIMPLEMENTED" << std::endl;
	}

	static void debug() {
		std::cout << "debug" << " UNIMPLEMENTED" << std::endl;
	}

	static void program_all_registers() {
		std::cout << "program_all_registers" << " UNIMPLEMENTED" << std::endl;
	}

	static void control_HB_command() {
		std::cout << "control_HB_command" << " UNIMPLEMENTED" << std::endl;
	}

	static void dump_all_registers() {
		std::cout << "dump_all_registers" << " UNIMPLEMENTED" << std::endl;
	}

	static void configure_frequency_sweep() {
		std::cout << "configure_frequency_sweep" << " UNIMPLEMENTED" << std::endl;
	}

	static void frequency_sweep_configured() {
		std::cout << "frequency_sweep_configured" << " UNIMPLEMENTED" << std::endl;
	}

	static void frequency_sweep_cleanup() {
		std::cout << "frequency_sweep_cleanup" << " UNIMPLEMENTED" << std::endl;
	}

	static void initialize_with_start_freq() {
		std::cout << "initialize_with_start_freq" << " UNIMPLEMENTED" << std::endl;
	}

	static void with_start_freq_initialized() {
		std::cout << "with_start_freq_initialized" << " UNIMPLEMENTED" << std::endl;
	}

	static void start_frequency_sweep() {
		std::cout << "start_frequency_sweep" << " UNIMPLEMENTED" << std::endl;
	}

	static void frequency_sweep_started() {
		std::cout << "frequency_sweep_started" << " UNIMPLEMENTED" << std::endl;
	}

	static void start_frequency_sweep_loop() {
		std::cout << "start_frequency_sweep_loop" << " UNIMPLEMENTED" << std::endl;
	}

	static void frequency_sweep_loop_top() {
		std::cout << "frequency_sweep_loop_top" << " UNIMPLEMENTED" << std::endl;
	}

	static void check_for_data_valid() {
		std::cout << "check_for_data_valid" << " UNIMPLEMENTED" << std::endl;
	}

	static void read_data_valid_bool() {
		std::cout << "read_data_valid_bool" << " UNIMPLEMENTED" << std::endl;
	}

	static void data_valid_bool_sent() {
		std::cout << "data_valid_bool_sent" << " UNIMPLEMENTED" << std::endl;
	}

	static void read_data_valid_value() {
		std::cout << "read_data_valid_value" << " UNIMPLEMENTED" << std::endl;
	}

	static void data_valid_value_sent() {
		std::cout << "data_valid_value_sent" << " UNIMPLEMENTED" << std::endl;
	}

	static void repeat_freq() {
		std::cout << "repeat_freq" << " UNIMPLEMENTED" << std::endl;
	}

	static void read_frequency_sweep_complete_bool() {
		std::cout << "read_frequency_sweep_complete_bool" << " UNIMPLEMENTED" << std::endl;
	}

	static void frequency_sweep_complete_bool_sent() {
		std::cout << "frequency_sweep_complete_bool_sent" << " UNIMPLEMENTED" << std::endl;
	}

	static void increment_frequency() {
		std::cout << "increment_frequency" << " UNIMPLEMENTED" << std::endl;
	}

	static void repeat_frequency_sweep() {
		std::cout << "repeat_frequency_sweep" << " UNIMPLEMENTED" << std::endl;
	}

	static void stop_frequency_sweep() {
		std::cout << "stop_frequency_sweep" << " UNIMPLEMENTED" << std::endl;
	}
};

namespace States {
	using sF = StateFunctions;
	using jPair = std::pair<decltype(MagicPackets::Debug::Command::start)*, const bState*>;
	using jPairo = std::optional<jPair>;
	using jPairoArray = std::array<jPairo, MAX_NUM_OF_JUMPS>;
	
	constinit const sState off {
		&sF::off
	};

	constinit const sState on {
		&sF::on
	};

	constinit const iState advertise {
		&sF::advertise,
		&advertising
	};
	
	constinit const sState advertising {
		&sF::advertising
	};

	constinit const iState connect {
		&sF::connect,
		&connected
	};

	constinit const eState connected {
		&sF::connected,
		jPairoArray {
			jPairo {
				jPair { &MagicPackets::Debug::Command::start, &debug }
			},
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::configure, &configure_frequency_sweep }
			}
		},
		&disconnect
	};

	constinit const iState disconnect {
		&sF::disconnect,
		&advertise
	};

	constinit const eState debug {
		&sF::debug,
		jPairoArray {
			jPairo {
				jPair { &MagicPackets::Debug::Command::dump_all_registers, &dump_all_registers }
			},
			jPairo {
				jPair { &MagicPackets::Debug::Command::program_all_registers, &program_all_registers }
			},
			jPairo {
				jPair { &MagicPackets::Debug::Command::control_HB_command, &control_HB_command }
			},
			jPairo {
				jPair { &MagicPackets::Debug::Command::end, &connected }
			}
		},
		&disconnect
	};

	constinit const iState program_all_registers {
		&sF::program_all_registers,
		&debug
	};

	constinit const iState dump_all_registers {
		&sF::dump_all_registers,
		&debug
	};

	constinit const iState control_HB_command {
		&sF::control_HB_command,
		&debug
	};

	constinit const iState configure_frequency_sweep {
		&sF::configure_frequency_sweep,
		&frequency_sweep_configured
	};

	constinit const eState frequency_sweep_configured {
		&sF::frequency_sweep_configured,
		jPairoArray {
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::initialize_with_start_freq, &initialize_with_start_freq }
			},
			jPairo {
				jPair { &MagicPackets::Debug::Command::start, &debug }
			}
		},
		&frequency_sweep_cleanup
	};

	constinit const iState frequency_sweep_cleanup {
		&sF::frequency_sweep_cleanup,
		&connected
	};

	constinit const iState initialize_with_start_freq {
		&sF::initialize_with_start_freq,
		&with_start_freq_initialized
	};

	constinit const eState with_start_freq_initialized {
		&sF::with_start_freq_initialized,
		jPairoArray {
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::start, &start_frequency_sweep }
			},
			jPairo {
				jPair { &MagicPackets::Debug::Command::start, &debug }
			}
		},
		&frequency_sweep_cleanup
	};

	constinit const iState start_frequency_sweep {
		&sF::start_frequency_sweep,
		&frequency_sweep_loop_top
	};

	constinit const eState frequency_sweep_loop_top {
		&sF::frequency_sweep_loop_top,
		jPairoArray {
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::check_for_data_valid, &check_for_data_valid }
			},
			jPairo {
				jPair { &MagicPackets::Debug::Command::start, &debug }
			}
		},
		&frequency_sweep_cleanup
	};

	constinit const iState check_for_data_valid {
		&sF::check_for_data_valid,
		&read_data_valid_bool
	};

	constinit const iState read_data_valid_bool {
		&sF::read_data_valid_bool,
		&data_valid_bool_sent
	};

	constinit const eState data_valid_bool_sent {
		&sF::data_valid_bool_sent,
		jPairoArray {
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::read_data_valid_value, &read_data_valid_value }
			},
			jPairo {
				jPair { &MagicPackets::Debug::Command::start, &debug }
			}
		},
		&frequency_sweep_cleanup
	};

	constinit const iState read_data_valid_value {
		&sF::read_data_valid_value,
		&data_valid_value_sent
	};

	constinit const eState data_valid_value_sent {
		&sF::data_valid_value_sent,
		jPairoArray {
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::check_for_sweep_complete, &read_frequency_sweep_complete_bool }
			},
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::repeat_freq, &repeat_freq }
			},
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::stop_frequency_sweep, &stop_frequency_sweep }
			},
			jPairo {
				jPair { &MagicPackets::Debug::Command::start, &debug }
			}
		},
		&frequency_sweep_cleanup
	};

	constinit const iState repeat_freq {
		&sF::repeat_freq,
		&frequency_sweep_loop_top
	};

	constinit const iState read_frequency_sweep_complete_bool {
		&sF::read_frequency_sweep_complete_bool,
		&frequency_sweep_complete_bool_sent
	};

	constinit const eState frequency_sweep_complete_bool_sent {
		&sF::frequency_sweep_complete_bool_sent,
		jPairoArray {
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::increment_frequency, &increment_frequency }
			},
			jPair {
				jPair { &MagicPackets::FrequencySweep::Command::repeat_frequency_sweep, &repeat_frequency_sweep }
			},
			jPairo {
				jPair { &MagicPackets::FrequencySweep::Command::stop_frequency_sweep, &stop_frequency_sweep }
			},
			jPairo {
				jPair { &MagicPackets::Debug::Command::start, &debug }
			}
		},
		&frequency_sweep_cleanup
	};

	constinit const iState increment_frequency {
		&sF::increment_frequency,
		&frequency_sweep_loop_top
	};

	constinit const iState repeat_frequency_sweep {
		&sF::repeat_frequency_sweep,
		&start_frequency_sweep
	};

	constinit const iState stop_frequency_sweep {
		&sF::stop_frequency_sweep,
		&frequency_sweep_cleanup
	};
}
