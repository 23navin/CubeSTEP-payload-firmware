/**
 * @file telemetry.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Stores and manages telemetry data for the payload processor
**/

#ifndef _telemetry_H_included
#define _telemetry_H_included

#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <random> //for testing
#include "ESP32Time.h"

using namespace std;

//Char Length Definitions
#define number_of_temp_sensors  16  // Number of temperatures sensors being read by adc
#define number_of_watt_outs     2   // Number of discrete wattage outputs

#define cell_size_epoch         9   // Length of char* to hold each time value in .csv
#define cell_size_mSecond       4   // Length of char* to hold each time value in .csv
#define cell_size_temp          8   // Length of char* to hold each temp value in .csv
#define cell_size_watt          7   // Length of char* to hold each wattage value in .csv

#define type_size_time          (cell_size_epoch + cell_size_mSecond + 2)  // Length of char* to hold all time values in .csv
#define type_size_temp          (number_of_temp_sensors * cell_size_temp + number_of_temp_sensors)   // Length of char* to hold all temp values in .csv
#define type_size_watt          (number_of_watt_outs * cell_size_watt + number_of_watt_outs)   // Length of char* to hold all watt values in .csv

#define line_size               (type_size_time + type_size_temp + type_size_watt + 3)   // Length of char* to hold all telemetry in .csv

#define char_precision_temp     3   // Precision of char* to hold temp value in .csv
#define char_precision_watt     3   // Precision of char* to hold wattage value in .csv

/**
 * @brief Payload telemetry structure
 * @note - Contains time, temperatures, and wattage data
 * @note - Contains built-in functions to copy telemetry data to char[]s suitable for a .csv file
 */
struct Telemetry{
    /* member declarations */
    unsigned long Seconds; // time: Seconds portion
    unsigned int mSeconds; // time: micro-Seconds portion
    float Sens[number_of_temp_sensors]; // temperature array
    float Watt[number_of_watt_outs]; // wattage array; todo: change to duty cycle

    ESP32Time realtime; //for testing

    /* methods*/

    /**
     * @brief Construct a new Telemetry object
     * @note constructor calls clear() to set all member data types to 0
     * 
     */
    Telemetry();

    /**
     * @brief Copies full telemetry into a string
     * 
     * @param LineChar destination string. length must be [line_size]
     */
    void ToCSV(char *LineChar);

    /**
     * @brief Copies time telemetry into a string 
     * 
     * @param TimeChar destination string. length must be [type_size_time]
     */
    void TimeToCSV(char *TimeChar);

    /**
     * @brief Copies time members into distinct strings
     * 
     * @param SecondsChar destination string for whole number part. length must be [cell_size_time]
     * @param uSecondsChar destination string for fractional part. length must be [cell_size_time]
     */
    void TimeToCSV(char *SecondsChar, char*uSecondsChar);

    /**
     * @brief Copies temperature telemetry into a string
     * 
     * @param TempChar destination string. length must be [type_size_temp]
     */
    void TempToCSV(char *TempChar);

    /**
     * @brief Copies a temperature member into a string
     * 
     * @param CellChar destination string. length must be [cell_size_temp]
     * @param channel temperature sensor channel number. from [0] to [number_of_temp_sensors]
     */
    void TempToCSV(char *CellChar, int channel);

    /**
     * @brief Copies wattage telemetry into a string
     * 
     * @param WattChar destination string. length must be [type_size_watt]
     */
    void WattToCSV(char *WattChar);

    /**
     * @brief Copies a wattage member into a string
     * 
     * @param CellChar destination string. length must be [cell_size_watt]
     * @param channel wattage output channel number. from [0] to [number_of_watt_outs]
     */
    void WattToCSV(char *CellChar, int channel);

    /**
     * @brief Copies csv header template into a string
     * 
     * @param LineChar destination string. minimum length depends on number of temps and watt outs
     */
    void headerCSV(char *LineChar);

    /**
     * @brief Sets all member variables to 0
     * 
     */
    void clear();

    /**
     * @brief Sets all member variables to a random value
     * @note Uses rand()
     * 
     */
    void random(ESP32Time *realtime);

    /**
     * @brief Set a single temperature
     * 
     * @param sensor
     * @param temperature 
     */
    void setTemp(int sensor, float temperature);

    /**
     * @brief Set all temperatures
     * 
     * @param temp_array pointer to an array of temperatures
     */
    void setTemp(float *temp_array[]);

    /**
     * @brief Set time to realtime
     * 
     */
    void setTime();

    /**
     * @brief Set time to parameter
     * 
     * @param epoch seconds
     * @param milli milliseconds
     */
    void setTime(unsigned long epoch, unsigned int milli);


    //void import(String inputString);

    //void import(char *inputArray);
};
#endif // _telemetry_H_included