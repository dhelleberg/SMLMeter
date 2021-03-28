#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>
#include <list>
#include "SMLSensorConfig.h"
#include <sml/sml_file.h>

/*Put your SSID & Password*/
const char* ssid = WSSID;  // Enter SSID here
const char* password = WPWD;  //Enter Password here

const int WIFI_TIMEOUT_RESTART_S=60;

std::list<SMLSensor*> *sensors = new std::list<SMLSensor*>();

MqttConfig mqttConfig;
MqttPublisher publisher;


//callback if a SML message is received
void process_message(byte *buffer, size_t len, SMLSensor *sensor)
{
	// Parse
	sml_file *file = sml_file_parse(buffer + 8, len - 16);

	if (connected) {
		publisher.publish(sensor, file);
	}

	// free the malloc'd memory
	sml_file_free(file);
}


void setup() {
  Serial.begin(115200);
  Serial.printf("SSID: %s\n",ssid);
  // Setup reading heads
	Serial.printf("Setting up %d configured sensors...\n", NUM_OF_SENSORS);
	const SensorConfig *config  = SENSOR_CONFIGS;
	for (uint8_t i = 0; i < NUM_OF_SENSORS; i++, config++)
	{
		SMLSensor *sensor = new SMLSensor(config, process_message);
		sensors->push_back(sensor);
	}
	DEBUG("Sensor setup done.");

}

void loop() {
  // put your main code here, to run repeatedly:
}