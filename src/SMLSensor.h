#ifndef SMLSENSOR_H
#define SMLSENSOR_H

#include<SoftwareSerial.h>
#include <jled.h>

// SML constants
const byte START_SEQUENCE[] = {0x1B, 0x1B, 0x1B, 0x1B, 0x01, 0x01, 0x01, 0x01};
const byte END_SEQUENCE[] = {0x1B, 0x1B, 0x1B, 0x1B, 0x1A};
const size_t BUFFER_SIZE = 3840; // Max datagram duration 400ms at 9600 Baud
const uint8_t READ_TIMEOUT = 30;

// States
enum State
{
  INIT,
  WAIT_FOR_START_SEQUENCE,
  READ_MESSAGE,
  PROCESS_MESSAGE,
  READ_CHECKSUM
};


class SensorConfig
{
public:
  const uint8_t pin;
  const char *name;
  const bool numeric_only;
  const bool status_led_enabled;
  const bool status_led_inverted;
  const uint8_t status_led_pin;
  const uint8_t interval;
};



class SMLSensor
{

public:
  const SensorConfig *config;
  SMLSensor(const SensorConfig *config, void (*callback)(byte *buffer, size_t len, SMLSensor *sensor));
  void loop();
  void process_message();

private:
  SoftwareSerial *serial;
  byte buffer[BUFFER_SIZE];
  size_t position = 0;
  unsigned long last_state_reset = 0;
  unsigned long last_callback_call = 0;
  uint8_t bytes_until_checksum = 0;
  uint8_t loop_counter = 0;
  State state = INIT;
  void (*callback)(byte *buffer, size_t len, SMLSensor *sensor) = NULL;
  JLed *status_led;

  void init_state();
  void run_current_state();
  void wait_for_start_sequence();
  void read_message();
  void read_checksum();  
  void reset_state(const char *message);
  int data_available();
  void set_state(State new_state);
  int data_read();

};

#endif // SMLSENSOR_H
