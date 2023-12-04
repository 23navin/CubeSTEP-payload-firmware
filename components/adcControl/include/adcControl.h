/**
 * @file Sensor.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Read temperatures into Telemetry
 * 
*/


#include "esp_log.h"
#include "esp_err.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "driver/gpio.h"

#include <stdio.h>
#include <math.h>

#ifndef _sensor_H_included
#define _sensor_H_included

namespace adcControl{
    //ADC config
    constexpr adc_atten_t adcAttenuation = ADC_ATTEN_DB_11;
    constexpr adc_bitwidth_t adcBitWidth = ADC_BITWIDTH_12;
    constexpr int numSamples = 5; //number of samples to be averaged for each adc output
    constexpr int numSensors = 16; //number of sensors to be sampled

    constexpr gpio_num_t adcPin = GPIO_NUM_18;

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
         * @brief Turn on GPIO power to thermistors
         * @note Power must be turned on for thermistors to read
         * @note Power needs to be on for at least 7 seconds for thermistor readings to stabilize.
         */
        void powerOn();

        /**
         * @brief Turn off GPIO power to thermistors
         * 
         */
        void powerOff();

        /**
         * @brief Single read of an ADC1 channel
         * 
         * @param value integer to hold read value
         * @param channel adc1_channel_t value
         */
        void readADC1(int *value_out, adc_channel_t channel);

        /**
         * @brief Single read of an ADC2 channel
         * 
         * @param value integer to hold read value
         * @param channel adc2_channel_t value
         */
        void readADC2(int *value_out, adc_channel_t channel);

        /**
         * @brief Samples adc at given sensor
         * 
         * @param sensor integer number of sensor; 0 -> 15
         * @return float temperature value in Kelvin
         */
        float sample(int sensor);

        float test();

    private:
        //should have a handle for each channel, but this seems to work fine
        adc_oneshot_unit_handle_t adc1_handle;
        adc_oneshot_unit_handle_t adc2_handle;
        
        adc_cali_handle_t cali_handle_unit1 = NULL;
        adc_cali_handle_t cali_handle_unit2 = NULL;
        
        const float r_inf = thermistorNominal*exp((-bCoefficient)/(kelvin+temperatureNominal)); //thermistor's resistance at nominal temperature

    };
}


#endif // _sensor_H_included