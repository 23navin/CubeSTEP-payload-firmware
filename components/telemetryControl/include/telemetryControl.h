/**
 * @file telemetry.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Stores and manages telemetry data for the payload processor
**/

#ifndef _telemetry_H_included
#define _telemetry_H_included

#include "esp_sntp.h"

#include <stdint.h>
#include <stdio.h>
#include <cstring>

using namespace std;

//Char Length Definitions
#define number_of_temp_sensors  16  // Number of temperatures sensors being read by adc

#define cell_size_epoch         11   // Length of char* to hold each time value in .csv
#define cell_size_mSecond       7   // Length of char* to hold each time value in .csv
#define cell_size_temp          8   // Length of char* to hold each temp value in .csv
#define cell_size_pwm_duty      4
#define cell_size_pwm_period    5

#define type_size_time          (cell_size_epoch + cell_size_mSecond + 2)  // Length of char* to hold all time values in .csv
#define type_size_temp          (number_of_temp_sensors * cell_size_temp + number_of_temp_sensors)   // Length of char* to hold all temp values in .csv
#define type_size_pwm           (cell_size_pwm_duty + cell_size_pwm_period)   // Length of char* to hold all pwm values in .csv

#define line_size               (type_size_time + type_size_temp + type_size_pwm + 3)   // Length of char* to hold all telemetry in .csv

#define char_precision_temp     3   // Precision of char* to hold temp value in .csv
#define char_precision_pwm      3   // Precision of char* to hold pwm duty values in .csv

/**
 * @brief Payload telemetry structure
 * @note - Contains time, temperatures, and pwm out data
 * @note - Contains built-in functions to copy telemetry data to char[]s suitable for a .csv file
 */
struct Telemetry{
    /* member declarations */
    unsigned long Seconds; // time: Seconds portion
    unsigned long uSeconds; // time: micro-Seconds portion
    float Sens[number_of_temp_sensors]; // temperature array
    int pwm_Duty; // pwm duty cycle (percentage)
    float pwm_Period; // pwm period length

    /* methods */

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
     * @brief Copies PWM telemetry into a string
     * 
     * @param PWMChar destination string. length must be [type_size_pwm]
     */
    void PWMToCSV(char *PWMChar);

    /**
     * @brief Copies a PWM member into a string
     * 
     * @param CellChar destination string. length must be [cell_size_pwm]
     * @param channel PWM output channel number. from [0] to [number_of_pwm_outs]
     */
    void PWMToCSV(char *DutyChar, char*PeriodChar);

    /**
     * @brief Copies csv header template into a string
     * 
     * @param LineChar destination string. minimum length depends on number of temps and pwm outs
     */
    void headerCSV(char *LineChar);

    /**
     * @brief Sets all member variables to 0
     * 
     */
    void clear();

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
     * @param micro micro-seconds
     */
    void setTime(unsigned long epoch, unsigned long micro);

    void setPWM(int duty_percentage, float cycle_period);
};
#endif // _telemetry_H_included