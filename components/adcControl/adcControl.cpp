/**
 * @file Sensor.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implemetation of SensorCore class
**/

#include "adcControl.h"
static const char* TAG = "SensorCore";

SensorCore::SensorCore(){
    //Set up ADC units
    esp_adc_cal_characterize(ADC_UNIT_1, _adc_attenuation, ADC_WIDTH_BIT_12, ESP_ADC_CAL_VAL_DEFAULT_VREF, &adc1_chars);
    esp_adc_cal_characterize(ADC_UNIT_2, _adc_attenuation, ADC_WIDTH_BIT_12, ESP_ADC_CAL_VAL_DEFAULT_VREF, &adc2_chars);
    adc_set_data_inv((adc_unit_t)3, true); //inverts both adcs; unsure why this is needed :|

    // adc_power_acquire();
    
}

SensorCore::~SensorCore(){
    ESP_LOGI(TAG, "POWER ADC OFF");
    // adc_power_release();
}

void SensorCore::readADC1(int *value_out, adc1_channel_t channel){
    int buffer;
    
    //read ADC
    adc1_get_raw(channel); //ignore first reading
    buffer = adc1_get_raw(channel);
    ESP_LOGV(TAG, "ADC1(%i) read at %i", (int)channel, (int)buffer);

    //write to provided int var
    *value_out = buffer;
}

void SensorCore::readADC2(int *value_out, adc2_channel_t channel){
    int buffer, ignore;

    //read ADC
    adc2_get_raw(channel, ADC_WIDTH_BIT_12, &ignore); //ignore first reading
    adc2_get_raw(channel, ADC_WIDTH_BIT_12, &buffer);
    ESP_LOGV(TAG, "ADC2(%i) read at %i", (int)channel, (int)buffer);

    //write to provided int var
    *value_out = buffer;
}

float SensorCore::sample(int sensor){
    int adc_channel; //adc channel irrespective of adc unit
    uint32_t reading = 0; //adc read value
    uint32_t average_reading; //average of samples
    uint32_t voltage; //read value converted to voltage value
    float resistance; //calculated resistance of thermistor given voltage
    float temperature; //calculated temperature from thermistor's resistance

    //Convert sensor value to channel value considering the appropriate unit and unsused adc channels
    if(sensor > 9) adc_channel = sensor - 6;
    else if(sensor > 8) adc_channel = sensor - 7; //skip adc2_1 if using adc2_0
    else if(sensor > 7) adc_channel = sensor - 8; //reset count to adc2 if using adc2_0
    //else if(sensor > 7) adc_channel = sensor - 7; //reset count to adc2 and skip adc2_0 if using adc2_1
    else adc_channel = sensor;
    
    //sample ADC loop
    for (int i = 0; i < num_of_samples; i++)
    {
        int buffer;

        if(sensor < 8) readADC1(&buffer, (adc1_channel_t)adc_channel); //if sensor0-7, use adc1
        else readADC2(&buffer, (adc2_channel_t)adc_channel); //if sensor 8-15, use adc2

        reading += buffer; //add sample to loop sum
    } //sampling loop
    average_reading = reading / num_of_samples; //divide loop sum by number of samples to find average sample reading

    //Convert sample reading to a voltage (mV) using adc characteristics
    if(sensor < 8) voltage = esp_adc_cal_raw_to_voltage(average_reading, &adc1_chars);
    else voltage = esp_adc_cal_raw_to_voltage(average_reading, &adc2_chars);
    
    //Convert voltage to temperature (K) usin thermistor characteristics
    resistance = ((THERMISTORNOMINAL*SUPPLYVOLTAGE)/voltage)-THERMISTORNOMINAL;
    temperature = (BCOEFFICIENT/log(resistance/r_inf))-KELVIN;
    ESP_LOGD(TAG, "Sensor %i sampled at %f", (int)sensor, (float)temperature);


    return temperature;
}

float SensorCore::test(){
    float average = 0;

    for(int sensor = 0; sensor < num_of_sensors; sensor++){
        float buffer = sample(sensor);
        ESP_LOGV(TAG, "Sensor %i @ %f", (int)sensor, (float)buffer);
        average += buffer;
    }

    average /= num_of_sensors;
    ESP_LOGD(TAG, "SensorCore tested. Average temperature: %f", (float)average);

    return average;
}