#include <SoftwareSerial.h>
#include <SMLSensor.h>
#include <jled.h>


SMLSensor::SMLSensor(const SensorConfig *config, void (*callback)(byte *buffer, size_t len, SMLSensor *sensor))
{
  this->config = config;
  Serial.printf("Initializing sensor %s...\n", this->config->name);
  this->callback = callback;
  this->serial = new SoftwareSerial();
  this->serial->begin(9600, SWSERIAL_8N1, this->config->pin, -1, false);
  this->serial->enableTx(false);
  this->serial->enableRx(true);
  Serial.printf("Initialized sensor %s.\n", this->config->name);

  if (this->config->status_led_enabled)
  {
    this->status_led = new JLed(this->config->status_led_pin);
    if (this->config->status_led_inverted)
    {
      this->status_led->LowActive();
    }
  }

  this->init_state();
}

void SMLSensor::loop()
{
  this->run_current_state();
  yield();
  if (this->config->status_led_enabled)
  {
    this->status_led->Update();
    yield();
  }
}


void SMLSensor::run_current_state()
{
  if (this->state != INIT)
  {
    if ((millis() - this->last_state_reset) > (READ_TIMEOUT * 1000))
    {
      Serial.printf("Did not receive an SML message within %d seconds, starting over.\n", READ_TIMEOUT);
      this->reset_state("");
    }
    switch (this->state)
    {
    case WAIT_FOR_START_SEQUENCE:
      this->wait_for_start_sequence();
      break;
    case READ_MESSAGE:
      this->read_message();
      break;
    case PROCESS_MESSAGE:
      this->process_message();
      break;
    case READ_CHECKSUM:
      this->read_checksum();
      break;
    default:
      break;
    }
  }
}

// Wrappers for sensor access
int SMLSensor::data_available()
{
  return this->serial->available();
}
int SMLSensor::data_read()
{
  return this->serial->read();
}

// Set state
void SMLSensor::set_state(State new_state)
{
  if (new_state == WAIT_FOR_START_SEQUENCE)
  {
    Serial.printf("State of sensor %s is 'WAIT_FOR_START_SEQUENCE'.\n", this->config->name);
    this->last_state_reset = millis();
    this->position = 0;
  }
  else if (new_state == READ_MESSAGE)
  {
    Serial.printf("State of sensor %s is 'READ_MESSAGE'.\n", this->config->name);
  }
  else if (new_state == READ_CHECKSUM)
  {
    Serial.printf("State of sensor %s is 'READ_CHECKSUM'.\n", this->config->name);
    this->bytes_until_checksum = 3;
  }
  else if (new_state == PROCESS_MESSAGE)
  {
    Serial.printf("State of sensor %s is 'PROCESS_MESSAGE'.\n", this->config->name);
  };
  this->state = new_state;
}

// Initialize state machine
void SMLSensor::init_state()
{
  this->set_state(WAIT_FOR_START_SEQUENCE);
}

// Start over and wait for the start sequence
void SMLSensor::reset_state(const char *message = NULL)
{
  if (message != NULL && strlen(message) > 0)
  {
    Serial.printf(message);
  }
  this->init_state();
}

// Wait for the start_sequence to appear
void SMLSensor::wait_for_start_sequence()
{
  while (this->data_available())
  {
    this->buffer[this->position] = this->data_read();
    yield();

    this->position = (this->buffer[this->position] == START_SEQUENCE[this->position]) ? (this->position + 1) : 0;
    if (this->position == sizeof(START_SEQUENCE))
    {
      // Start sequence has been found
      Serial.println("Start sequence found.");
      if (this->config->status_led_enabled)
      {
        this->status_led->Blink(50, 50).Repeat(3);
      }
      this->set_state(READ_MESSAGE);
      return;
    }
  }
}

// Read the rest of the message
void SMLSensor::read_message()
{
  while (this->data_available())
  {
    // Check whether the buffer is still big enough to hold the number of fill bytes (1 byte) and the checksum (2 bytes)
    if ((this->position + 3) == BUFFER_SIZE)
    {
      this->reset_state("Buffer will overflow, starting over.");
      return;
    }
    this->buffer[this->position++] = this->data_read();
    yield();

    // Check for end sequence
    int last_index_of_end_seq = sizeof(END_SEQUENCE) - 1;
    for (int i = 0; i <= last_index_of_end_seq; i++)
    {
      if (END_SEQUENCE[last_index_of_end_seq - i] != this->buffer[this->position - (i + 1)])
      {
        break;
      }
      if (i == last_index_of_end_seq)
      {
        Serial.println("End sequence found.");
        this->set_state(READ_CHECKSUM);
        return;
      }
    }
  }
}

// Read the number of fillbytes and the checksum
void SMLSensor::read_checksum()
{
  Serial.println("reading checksum");
  while (this->bytes_until_checksum > 0 && this->data_available())
  {
    this->buffer[this->position++] = this->data_read();
    this->bytes_until_checksum--;
    yield();
  }

  if (this->bytes_until_checksum == 0)
  {
    Serial.printf("Message has been read.");
    //DEBUG_DUMP_BUFFER(this->buffer, this->position);
    this->set_state(PROCESS_MESSAGE);
  }
  else
    Serial.println("checksum NOT OK");
  this->set_state(PROCESS_MESSAGE);
}

void SMLSensor::process_message()
{
  Serial.println("Message is being processed.");

  // Call listener
  if (this->callback != NULL)
  {
    if (this->config->interval == 0 || ((millis() - this->last_callback_call) > (this->config->interval * 1000)))
    {

      this->last_callback_call = millis();
      this->callback(this->buffer, this->position, this);
    }
  }

  // Start over
  this->reset_state("");
}
