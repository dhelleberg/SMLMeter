#include "BME280Sensor.h"

void BME280Sensor::setup(void (*callback)(float temperature, float pressure, float humidty )) {
  this->callback = callback;
  if (! bme.begin(0x76, &Wire)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }  
  Serial.println("normal mode, 16x oversampling for all, filter off,");
  

}

void BME280Sensor::loop() {
  long now = millis();
  if(now > (lastMeasure + CACHE_TIME)) {
    lastMeasure = now;

    const float scale = 10.0; 
    double temp = ((float)((int)(bme.readTemperature() * scale))) / scale;
    double pressure = ((float)((int)(bme.readPressure()/100.0 * scale))) / scale; 
    double humidity = ((float)((int)(bme.readHumidity() * scale))) / scale; 

    if(humidity != lasthumidty || temp != lastTemp || pressure != lastpressure) {
      Serial.println("measure updates, callback called");
      this->lasthumidty = humidity;
      this->lastTemp = temp;
      this->lastpressure = pressure;

      // Call listener
      if (this->callback != NULL) {
        this->callback(temp, pressure, humidity);
      }
    }
  
  }
}


