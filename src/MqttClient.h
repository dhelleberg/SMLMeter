#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include "MQTT.h"
#include <sml/sml_file.h>
#include <map>

struct MqttConfig
{
  char server[128] = "192.168.178.21";
  char port[8] = "1883";
  char username[128] = "";
  char password[128] = "";
  char topic[128] = "iot/cellar/";
};


class MqttClient
{
  public:
    void setup(MqttConfig _config);
    void connect();
    void publish(SMLSensor *sensor, sml_file *file);
    void loop();
    void publishDebug(const char *message);
    void publishInfo(const char *message);

    

  private:
    void publish(const String &topic, const String &payload);
    void publish(String &topic, const char *payload);
    void publish(const char *topic, const String &payload);
    void publish(const char *topic, const char *payload);
    MqttConfig config;
    MQTTClient client = MQTTClient(512);
    String baseTopic;
    WiFiClient net;
    std::map<String, String> cache;
};








#endif // MQTTCLIENT_H
