#ifndef SMLSENSORCONFIG_H
#define SMLSENSORCONFIG_H

#include "Arduino.h"
#include "SMLSensor.h"

static const SensorConfig SENSOR_CONFIGS[] = {
    {.pin = D2,
     .name = "smeter/",
     .numeric_only = false,
     .status_led_enabled = true,
     .status_led_inverted = true,
     .status_led_pin = LED_BUILTIN,
     .interval = 0}
};

const uint8_t NUM_OF_SENSORS = sizeof(SENSOR_CONFIGS) / sizeof(SensorConfig);
#endif // SMLSENSORCONFIG_H
