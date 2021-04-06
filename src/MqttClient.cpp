#include "config.h"
#include "MQTT.h"
#include "SMLSensor.h"
#include <string.h>
#include <WiFi.h>
#include <sml/sml_file.h>
#include "MqttClient.h"

void MqttClient::setup(MqttConfig _config)
{
  Serial.println("Setting up MQTT publisher.");
  config = _config;
  uint8_t lastCharOfTopic = strlen(config.topic) - 1;
  baseTopic = String(config.topic) + (lastCharOfTopic >= 0 && config.topic[lastCharOfTopic] == '/' ? "" : "/");

  client.begin(config.server, atoi(config.port), net);
}

void MqttClient::connect()
{
  Serial.println("Establishing MQTT client connection.");
  client.connect("SMLReader", config.username, config.password);
  if (client.connected())
  {
    Serial.printf("Hello running SMLReader version %s.", FIRMWARE_VERSION);
    publishInfo("SML Reader online.");
    publishInfo(FIRMWARE_VERSION);
  }
}

void MqttClient::loop()
{
  client.loop();
}

void MqttClient::publishDebug(const char *message)
{
  publish(baseTopic + "debug", message);
}

void MqttClient::publishInfo(const char *message)
{
  publish(baseTopic + "info", message);
}

void MqttClient::publish(SMLSensor *sensor, sml_file *file)
{

  for (int i = 0; i < file->messages_len; i++)
  {
    sml_message *message = file->messages[i];
    if (*message->message_body->tag == SML_MESSAGE_GET_LIST_RESPONSE)
    {
      sml_list *entry;
      sml_get_list_response *body;
      body = (sml_get_list_response *)message->message_body->data;
      for (entry = body->val_list; entry != NULL; entry = entry->next)
      {
        if (!entry->value)
        { // do not crash on null value
          continue;
        }

        char obisIdentifier[32];
        char buffer[255];

        sprintf(obisIdentifier, "%d-%d:%d.%d.%d/%d",
                entry->obj_name->str[0], entry->obj_name->str[1],
                entry->obj_name->str[2], entry->obj_name->str[3],
                entry->obj_name->str[4], entry->obj_name->str[5]);

        String entryTopic = baseTopic + "sensor/" + (sensor->config->name) + "/obis/" + obisIdentifier + "/";

        if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) ||
            ((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED))
        {
          double value = sml_value_to_double(entry->value);
          int scaler = (entry->scaler) ? *entry->scaler : 0;
          int prec = -scaler;
          if (prec < 0)
            prec = 0;
          value = value * pow(10, scaler);
          sprintf(buffer, "%.*f", prec, value);
          String val = String(buffer);
          //check cache first
          auto entry = cache.find(entryTopic);
          if (entry == cache.end())
          {
            publish(entryTopic + "value", buffer);
            cache[entryTopic] = val;
            Serial.println("entry not cache but not identical");
          }
          else
          {
            String cacheValue = cache[entryTopic];
            if (cacheValue.compareTo(val) != 0)
            {
              publish(entryTopic + "value", buffer);
              cache[entryTopic] = val;
              Serial.println("entry in cache but not identical");
            }
            else
              Serial.println("entry in cache and identical");
          }
        }
        else if (!sensor->config->numeric_only)
        {
          if (entry->value->type == SML_TYPE_OCTET_STRING)
          {
            char *value;
            sml_value_to_strhex(entry->value, &value, true);
            String val = String(value);
            //check cache first
            auto entry = cache.find(entryTopic);
            if (entry == cache.end())
            {
              publish(entryTopic + "value", value);
              cache[entryTopic] = val;
              Serial.println("entry not cache but not identical");
            }
            else
            {
              String cacheValue = cache[entryTopic];
              if (cacheValue.compareTo(val) != 0)
              {
                publish(entryTopic + "value", value);
                cache[entryTopic] = val;
                Serial.println("entry in cache but not identical");
              }
              else
                Serial.println("entry in cache and identical");
            }

            free(value);
          }
          else if (entry->value->type == SML_TYPE_BOOLEAN)
          {
            publish(entryTopic + "value", entry->value->data.boolean ? "true" : "false");
          }
        }
      }
    }
  }
}

void MqttClient::publish(const String &topic, const String &payload)
{
  publish(topic.c_str(), payload.c_str());
}
void MqttClient::publish(String &topic, const char *payload)
{
  publish(topic.c_str(), payload);
}
void publish(const char *topic, const String &payload)
{
  publish(topic, payload.c_str());
}
void MqttClient::publish(const char *topic, const char *payload)
{
  if (!client.connected())
  {
    connect();
  }
  if (!client.connected())
  {
    // Something failed
    Serial.println("Connection to MQTT broker failed.");
    Serial.printf("Unable to publish a message to '%s'.", topic);
    return;
  }
  Serial.printf("Publishing message to '%s':", topic);
  Serial.printf("%s\n", payload);
  client.publish(topic, payload, true, 1);
}

void MqttClient::publishSensor(float temp, float pressure, float humidty) {

  if (!client.connected())
  {
    connect();
  }
  if (!client.connected())
  {
    // Something failed
    Serial.println("Connection to MQTT broker failed.");
    Serial.printf("Unable to publish a message");
    return;
  }
  char array[20];
  sprintf(array, "%f", temp);
  client.publish(baseTopic+"/temp", array, true, 1);
  sprintf(array, "%f", pressure);
  client.publish(baseTopic+"/pressure", array, true, 1);
  sprintf(array, "%f", humidty);
  client.publish(baseTopic+"/humidty", array, true, 1);

}
