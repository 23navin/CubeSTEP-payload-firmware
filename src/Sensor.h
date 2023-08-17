/**
 * @file Sensor.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Read temperatures into Telemetry
 * 
*/

#include "ESP32Time.h"
#include "Telemetry.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#ifndef _sensor_H_included
#define _sensor_H_included

//ADC settings
#define _adc_attenuation ADC_ATTEN_11db //Attenuance 
#define num_of_samples 5 //number of samples to be averages for each adc output

//Given thermistor values; reference: https://www.tme.eu/Document/f9d2f5e38227fc1c7d979e546ff51768/NTCM-100K-B3950.pdf
#define _BCOEFFICIENT 3950 //Beta Coefficient
#define _THERMISTORNOMINAL 100000 //Nominal Resistance; Value of series resistor
#define _TEMPERATURENOMINAL 25 //Nominal temperature in Celsius
#define _KELVIN 273.15 //Offset for calculation to convert Celsius to Kelvin 
#define _SUPPLYVOLTAGE 3.3 //Voltage supplied to thermistor in Volts (usually 3.3V or 5V)

//consider changing the following macros to const float
#define BCOEFFICIENT (float)_BCOEFFICIENT // Beta coefficient of thermistor
#define THERMISTORNOMINAL (float)_THERMISTORNOMINAL // Resistance of thermistor and matching resistor
#define SUPPLYVOLTAGE (float)(1000*_SUPPLYVOLTAGE) // Voltage (mV) supplied to thermistor
#define TEMPERATURENOMINAL (float)_TEMPERATURENOMINAL // Zero power resistance
#define KELVIN (float)_KELVIN // Offset to convert Celsius to Kelvin

/**
 * @brief Interface to read system thermistors
 * @note - Read ADC
 * @note - Convert readings to temperature values
 * @note - Send results to Telemetry object
 */
class SensorCore{
public:
    /**
     * @brief Construct a new Sensor Core object
     * @note Constructor sets time to time at compile
     * 
     */
    SensorCore();
    ~SensorCore();

    /**
     * @brief Initializes SensorCore. Must be called after system time has been configured
     * 
     * @param time realtime object
     */
    void init(ESP32Time *time);

    /**
     * @brief Single read of an ADC1 channel
     * 
     * @param value integer to hold read value
     * @param channel adc1_channel_t value
     */
    void readADC1(int *value_out, adc1_channel_t channel);

    /**
     * @brief Single read of an ADC2 channel
     * 
     * @param value integer to hold read value
     * @param channel adc2_channel_t value
     */
    void readADC2(int *value_out, adc2_channel_t channel);

    /**
     * @brief Samples adc at given sensor
     * 
     * @param sensor integer number of sensor; 0 -> 15
     * @return float temperature value in Kelvin
     */
    float sample(int sensor);

    /**
     * @brief Records all system temperature values
     * 
     * @param telemetry_out Telemetry object to hold temperatures and time 
     */
    void snapshot(Telemetry *telemetry_out);

private:
    ESP32Time realtime; //Time object to hold the current time that corresponds to the sensor's readings
    
    esp_adc_cal_characteristics_t adc1_chars; //Characteristic object for ADC1 (sensors 0-7)
    esp_adc_cal_characteristics_t adc2_chars; //Characteristic object for ADC2 (sensors 8-15)
    
    const float r_inf = THERMISTORNOMINAL*exp((-BCOEFFICIENT)/(KELVIN+TEMPERATURENOMINAL)); //thermistor's resistance at nominal temperature

};
#endif // _sensor_H_included