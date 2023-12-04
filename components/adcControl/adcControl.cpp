/**
 * @file Sensor.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implemetation of adc class
**/

#include "adcControl.h"
static const char* TAG = "adc";

adcControl::adc::adc(){
    //adc1 setup
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = adcAttenuation,
        .bitwidth = adcBitWidth
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config); //Sensor 0
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config); //Sensor 1
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &config); //Sensor 2
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config); //Sensor 3
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config); //Sensor 4
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_5, &config); //Sensor 5
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config); //Sensor 6
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &config); //Sensor 7

    adc_cali_line_fitting_config_t cali_config_unit1 = {
        .unit_id = ADC_UNIT_1,
        .atten = adcAttenuation,
        .bitwidth = adcBitWidth,
    };

    adc_cali_create_scheme_line_fitting(&cali_config_unit1, &cali_handle_unit1);

    //adc2 setup
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config2, &adc2_handle);

    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_0, &config); //Sensor 8
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_2, &config); //Sensor 9
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_4, &config); //Sensor 10
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_5, &config); //Sensor 11
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_6, &config); //Sensor 12
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_7, &config); //Sensor 13
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_8, &config); //Sensor 14
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_9, &config); //Sensor 15

    adc_cali_line_fitting_config_t cali_config_unit2 = {
        .unit_id = ADC_UNIT_2,
        .atten = adcAttenuation,
        .bitwidth = adcBitWidth,
    };

    adc_cali_create_scheme_line_fitting(&cali_config_unit2, &cali_handle_unit2);

    //gpio power
    gpio_config_t io_conf = {
        .pin_bit_mask = (uint64_t)0x1 << adcControl::adcPin,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE

    };
    gpio_config(&io_conf);
    gpio_set_level(adcControl::adcPin, 0);
}

adcControl::adc::~adc(){
    adc_oneshot_del_unit(adc1_handle);
    adc_oneshot_del_unit(adc2_handle);

    adc_cali_delete_scheme_line_fitting(cali_handle_unit1);
    adc_cali_delete_scheme_line_fitting(cali_handle_unit2);

    gpio_set_level(adcControl::adcPin, 0);

    ESP_LOGI(TAG, "POWER ADC OFF");
    // adc_power_release();
}

void adcControl::adc::powerOn(){
    gpio_set_level(adcControl::adcPin, 1);
    ESP_LOGI(TAG, "Thermister Power On");
}

void adcControl::adc::powerOff(){
    gpio_set_level(adcControl::adcPin, 0);
    ESP_LOGI(TAG, "Thermister Power Off");
}

void adcControl::adc::readADC1(int *value_out, adc_channel_t channel){
    int ignore, buffer;
    
    //read ADC
    adc_oneshot_read(adc1_handle, channel, &ignore); //ignore first reading
    adc_oneshot_read(adc1_handle, channel, &buffer);
    ESP_LOGV(TAG, "ADC1(%i) read at %i", (int)channel, (int)buffer);

    //write to provided int var
    *value_out = buffer;
    
}

void adcControl::adc::readADC2(int *value_out, adc_channel_t channel){
    int buffer, ignore;

    //read ADC
    adc_oneshot_read(adc2_handle, channel, &ignore); //ignore first reading
    adc_oneshot_read(adc2_handle, channel, &buffer);
    ESP_LOGV(TAG, "ADC2(%i) read at %i", (int)channel, (int)buffer);

    //write to provided int var
    *value_out = buffer;
}

float adcControl::adc::sample(int sensor){
    int adc_channel; //adc channel irrespective of adc unit
    uint32_t reading = 0; //adc read value
    uint32_t average_reading; //average of samples
    int voltage; //read value converted to voltage value
    float resistance; //calculated resistance of thermistor given voltage
    float temperature; //calculated temperature from thermistor's resistance

    //Convert sensor value to channel value considering the appropriate unit and unsused adc channels
    if(sensor > 9) adc_channel = sensor - 6;
    else if(sensor > 8) adc_channel = sensor - 7; //skip adc2_1 if using adc2_0
    else if(sensor > 7) adc_channel = sensor - 8; //reset count to adc2 if using adc2_0
    //else if(sensor > 7) adc_channel = sensor - 7; //reset count to adc2 and skip adc2_0 if using adc2_1
    else adc_channel = sensor;
    
    //sample ADC loop
    for (int i = 0; i < numSamples; i++)
    {
        int buffer;

        if(sensor < 8) readADC1(&buffer, (adc_channel_t)adc_channel); //if sensor0-7, use adc1
        else readADC2(&buffer, (adc_channel_t)adc_channel); //if sensor 8-15, use adc2
        reading += buffer; //add sample to loop sum
    } //sampling loop
    average_reading = reading / numSamples; //divide loop sum by number of samples to find average sample reading

    //Convert sample reading to a voltage (mV) using adc characteristics
    if(sensor < 8) adc_cali_raw_to_voltage(cali_handle_unit1, average_reading, &voltage);
    else adc_cali_raw_to_voltage(cali_handle_unit2, average_reading, &voltage);
    
    //Convert voltage to temperature (K) usin thermistor characteristics
    resistance = ((thermistorNominal*supplyVoltage)/voltage)-thermistorNominal;
    temperature = (bCoefficient/log(resistance/r_inf))-kelvin;
    ESP_LOGD(TAG, "Sensor %i sampled at %f", (int)sensor, (float)temperature);


    return temperature;
}

float adcControl::adc::test(){
    float average = 0;

    for(int sensor = 0; sensor < numSensors; sensor++){
        float buffer = sample(sensor);
        ESP_LOGV(TAG, "Sensor %i @ %f", (int)sensor, (float)buffer);
        average += buffer;
    }

    average /= numSensors;
    ESP_LOGD(TAG, "SensorCore tested. Average temperature: %f", (float)average);

    return average;
}
