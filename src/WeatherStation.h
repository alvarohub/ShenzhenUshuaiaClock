#ifndef WEATHER_STATION_H
#define WEATHER_STATION_H

// Weather station structure
struct WeatherStation {
  const char* name;
  float lat;
  float lon;
  const char* timezone;
  float temperature;
  float humidity;
  float dewPoint;
};

#endif // WEATHER_STATION_H
