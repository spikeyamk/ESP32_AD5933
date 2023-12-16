#pragma once

#include <cstdint>

#include "host/ble_uuid.h"

namespace BLE {
    namespace UUIDs {
        constexpr uint16_t APPEARANCE_GENERIC_SENSOR_UUID = 0x0540;
        constexpr uint16_t PROFILE_BODY_COMPOSITION_UUID = 0x1014;
        constexpr uint16_t CHARACTERISTIC_BODY_COMPOSITION_FEATURE_UUID = 0x2A9B;
        constexpr uint16_t CHARACTERISTIC_BODY_COMPOSITION_MEASUREMENT_UUID = 0x2A9C;
        constexpr uint16_t CONFIG_EXAMPLE_IO_TYPE = 3;
        //constexpr uint8_t SERVICE_BODY_COMPOSITION_UUID[2] = { 0x18, 0x1B };
        constexpr uint16_t SERVICE_BODY_COMPOSITION_UUID = 0x181B;

        /*
        constexpr uint16_t UNIT_ELECTRICAL_RESISTANCE_OHM_UUID = 0x272A;
        constexpr uint16_t UNIT_PLANE_ANGLE_DEGREE_UUID = 0x2763;
        constexpr uint16_t UNIT_PLANE_ANGLE_MINUTE_UUID = 0x2764;
        constexpr uint16_t UNIT_PLANE_ANGLE_RADIAN_UUID = 0x2720;
        constexpr uint16_t UNIT_PLANE_ANGLE_SECOND_UUID = 0x2765;
        */
    }
}