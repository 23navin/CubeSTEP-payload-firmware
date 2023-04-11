/**
 * @file telemetry.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief stores and manages telemetry data for the payload processor
**/

#ifndef _telemetry_H_included
#define _telemetry_H_included

#include <stdint.h>
#include <stdio.h>
#include <cstring>

/* definitions */
#define number_of_temp_sensors  16  // Number of temperatures sensors being read by adc
#define number_of_watt_outs     2   // Number of discrete wattage outputs

#define char_length_temp        8   // Length of char* to hold temp value in .csv
#define char_precision_temp     3   // Precision of char* to hold temp value in .csv
#define char_length_watt        7   // Length of char* to hold wattage value in .csv
#define char_precision_watt     3   // Precision of char* to hold wattage value in .csv

typedef int8_t                  i8;     // 8-bit signed integer
typedef uint8_t                 u8;     // 8-bit unsigned integer
typedef int16_t                 i16;    // 16-bit signed integer
typedef uint16_t                u16;    // 16-bit unsigned integer
typedef int32_t                 i32;    // 32-bit signed integer
typedef uint32_t                u32;    // 32-bit unsigned integer

struct Telemetry{
    //stored data
    float SENS[number_of_temp_sensors]; // temperature array
    float Watt[number_of_watt_outs];  // wattage array
    u32 seconds;    // time: seconds portion
    u32 useconds;   // time: micro-seconds portion

    Telemetry();

    //output to char* functions

    void toCSV(char *tempChar, int channel);

    void tocSV(char *wattChar, int channel);

    //set functions

    void setTime(u32 seconds, u32 useconds);

    void set(float newTemps[number_of_temp_sensors], float newWatt[2], u32 seconds, u32 useconds);

    void set(float newTemps[number_of_temp_sensors], float newWatt[2]);

    void clear();
};
#endif // _telemetry_H_included