#ifndef BME280SENSOR_H
#define BME280SENSOR_H

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

const long CACHE_TIME = 1000 * 10; //10 sec


class BME280Sensor
 {

   public:
    void setup(void (*callback)(float temperature, float pressure, float humidty ));
    void loop();

  private:
    void( *callback)(float temperature, float pressure, float humidty ) = NULL;
    Adafruit_BME280 bme; // I2C
    
    double lastTemp = 0;
    double lastpressure = 0;
    double lasthumidty = 0;
    long lastMeasure = 0;

 };


#endif // BME280SENSOR_H
