/**
 * @file Sensor.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Read temperatures into Telemetry
 * 
*/


#include "esp_log.h"
#include "esp_err.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <stdio.h>
#include <math.h>

#ifndef _sensor_H_included
#define _sensor_H_included

namespace adcControl{
    //ADC config
    constexpr adc_atten_t adcAttenuation = ADC_ATTEN_DB_11;
    constexpr int numSamples = 5; //number of samples to be averaged for each adc output
    constexpr int numSensors = 16; //number of sensors to be sampled

    //Given thermistor values; reference: https://www.tme.eu/Document/f9d2f5e38227fc1c7d979e546ff51768/NTCM-100K-B3950.pdf
    constexpr float bCoefficient = 3950; //Beta Coefficient
    constexpr float thermistorNominal = 100000; //Nominal resistance; Value of series resistor
    constexpr float temperatureNominal = 25; //Nominal temperature in Celsius
    constexpr float kelvin = 273.15; //Offset for calculation to convert Celsius to Kelvin
    constexpr float supplyVoltage = 3300; //Voltage supplied to thermistor in millivolts

    /**
     * @brief Interface to read system thermistors
     * @note - Read ADC
     * @note - Convert readings to temperature values
     * @note - Send results to Telemetry object
     */
    class adc{
    public:
        /**
         * @brief Construct a new Sensor Core object
         * @note Constructor sets time to time at compile
         * 
         */
        adc();
        ~adc();

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

        float test();

    private:
        
        esp_adc_cal_characteristics_t adc1_chars; //Characteristic object for ADC1 (sensors 0-7)
        esp_adc_cal_characteristics_t adc2_chars; //Characteristic object for ADC2 (sensors 8-15)
        
        const float r_inf = thermistorNominal*exp((-bCoefficient)/(kelvin+temperatureNominal)); //thermistor's resistance at nominal temperature

    };
}


#endif // _sensor_H_included