#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>
#include <list>
#include "SMLSensorConfig.h"
#include <sml/sml_file.h>
#include "MqttClient.h"
#include "BME280Sensor.h"

/*Put your SSID & Password*/
const char* ssid = WSSID;  // Enter SSID here
const char* password = WPWD;  //Enter Password here

const int WIFI_TIMEOUT_RESTART_S=60;

const int RECONNECT_INTERVAL = 30 * 1000;

std::list<SMLSensor*> *sensors = new std::list<SMLSensor*>();

MqttConfig mqttConfig;
MqttClient mqttClient;
long lastReconnect =  0;

BME280Sensor bme280Sensor;





//callback if a SML message is received
void process_message(byte *buffer, size_t len, SMLSensor *sensor)
{
	// Parse
	sml_file *file = sml_file_parse(buffer + 8, len - 16);


  mqttClient.publish(sensor, file);


	// free the malloc'd memory
	sml_file_free(file);
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting to WiFi ");

  int wifi_connection_time = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    wifi_connection_time++;
    if (wifi_connection_time > WIFI_TIMEOUT_RESTART_S) {
      Serial.println("[WIFI] Run into Wifi Timeout, try restart");
      Serial.flush();
      WiFi.disconnect();
      ESP.restart();
    }
  }
  Serial.println("\n[WIFI] Connected");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  lastReconnect = millis(); 
}

void process_sensorReading(float temp, float pressure, float humidty) {
  Serial.printf("new sensor reading from bme... temp: %f pressure %f humidty %f publishing...\n",temp,pressure,humidty);
  mqttClient.publishSensor(temp, pressure, humidty);

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
	Serial.println("Sensor setup done.");

  connectWifi();

  Serial.println("Wifi connected.");

  //connect mqtt
  Serial.println("Setup mqtt...");

  mqttClient.setup(mqttConfig);
  mqttClient.connect();
  
  Serial.println("mqtt connected");

  Serial.println("setup bme280...");
  bme280Sensor.setup(process_sensorReading);
  Serial.println("bme280 done.");

}

void checkWifi(long now) {
  if(now > (lastReconnect + RECONNECT_INTERVAL)) {
    lastReconnect = now;
    if(WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WIFI re-connecting...");
      int wifi_retry = 0;
      while(WiFi.status() != WL_CONNECTED && wifi_retry < 5 ) {
        wifi_retry++;
        Serial.println("WiFi not connected. Try to reconnect");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        delay(100);
      }
    }
  }
}

void loop() {
  long now = millis();
  checkWifi(now);
  mqttClient.loop();
  yield();
  bme280Sensor.loop();
  yield();

  // Execute sensor state machines
	for (std::list<SMLSensor*>::iterator it = sensors->begin(); it != sensors->end(); ++it){
		(*it)->loop();
	}
  yield();
  
}