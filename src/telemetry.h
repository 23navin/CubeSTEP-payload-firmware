/**
 * @file telemetry.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief stores and manages telemetry data for the payload processor
**/

#ifndef _telemetry_H_included
#define _telemetry_H_included

#include <stdint.h>

/* definitions */
#define number_of_temp_sensors  16  // Number of temperatures sensors being read by adc
#define number_of_watt_outs     2   // Number of discrete wattage outputs

#define char_length_temp        7   // Length of char* to hold temp value in .csv "XXX.XXX"
#define char_length_watt        6   // Length of char* to hold wattage valye in .csv "XX.XXX"

typedef int8_t                  i8;     // 8-bit signed integer
typedef uint8_t                 u8;     // 8-bit unsigned integer
typedef int16_t                 i16;    // 16-bit signed integer
typedef uint16_t                u16;    // 16-bit unsigned integer
typedef int32_t                 i32;    // 32-bit signed integer
typedef uint32_t                u32;    // 32-bit unsigned integer

class Telemetry{
public:
    //get
    void getTemp(float* tempArray[number_of_temp_sensors]);

    void getTemp(char* tempChar[char_length_temp], int sensor);

    void getWatt(float* wattArray[number_of_watt_outs]);

    void getWatt(char* wattChar[char_length_watt]);

    void getTime(u32 new_seconds, u32 new_useconds);


    //use template to create a function to return an individual sensor value

    //set
    void setTemps(float newTemps[number_of_temp_sensors]);

    void setWatt(float newWatt[number_of_watt_outs]);

    void setTime(u32 seconds, u32 useconds);

    void set(float newTemps[number_of_temp_sensors], float newWatt[2], u32 seconds, u32 useconds);

    void set(float newTemps[number_of_temp_sensors], float newWatt[2]);

private:
    float SENS[16]; // temperature array
    float Watt[2];  // wattage array
    u32 seconds;    // time: seconds portion
    u32 useconds;   // time: micro-seconds portion

};
#endif // _telemetry_H_included