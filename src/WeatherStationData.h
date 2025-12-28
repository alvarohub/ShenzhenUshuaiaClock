#ifndef WEATHER_STATION_DATA_H
#define WEATHER_STATION_DATA_H

#include "WeatherStation.h"

// ============================================
// WEATHER STATIONS CONFIGURATION
// ============================================
// Edit this file to add/remove weather stations
// NUM_STATIONS is automatically calculated from the array size

WeatherStation stations[] = {
  {"Ilulissat", 69.2198, -51.0986, "America/Godthab", -2.0, 50.0, 0.0},                    // Greenland glacier
  {"El Calafate", -50.3375, -72.2647, "America/Argentina/Rio_Gallegos", -2.0, 50.0, 0.0},  // Patagonia glacier
  {"Hong Kong", 22.3193, 114.1694, "Asia/Hong_Kong", 26.0, 75.0, 0.0},                     // Humid reference
  {"Shenzhen", 22.5431, 114.0579, "Asia/Shanghai", 14.0, 75.0, 0.0}                        // Local station
}; 

int NUM_STATIONS = sizeof(stations) / sizeof(stations[0]);  // Automatically calculated

// Station indices for easy reference
const int GLACIER_ILULISSAT = 0;
const int GLACIER_CALAFATE = 1;
const int HUMID_HONGKONG = 2;
const int LOCAL_SHENZHEN = 3;

#endif // WEATHER_STATION_DATA_H
