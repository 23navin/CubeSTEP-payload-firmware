/**
 * @file Sensor.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implemetation of SensorCore class
**/

#include "Sensor.h"

SensorCore::SensorCore(){
    //Set up ADC units
    esp_adc_cal_characterize(ADC_UNIT_1, _adc_attenuation, ADC_WIDTH_BIT_12, ESP_ADC_CAL_VAL_DEFAULT_VREF, &adc1_chars);
    esp_adc_cal_characterize(ADC_UNIT_2, _adc_attenuation, ADC_WIDTH_BIT_12, ESP_ADC_CAL_VAL_DEFAULT_VREF, &adc2_chars);
    adc_set_data_inv((adc_unit_t)3, true); //inverts both adcs; unsure why this is needed :|

    // adc_power_acquire();
}

SensorCore::~SensorCore(){
    log_i("POWER ADC OFF");
    // adc_power_release();
}

void SensorCore::init(ESP32Time *time){
    //get and set current time
    unsigned long epoch = time->getEpoch();
    unsigned int ms = time->getMillis();
    realtime.setTime(epoch, ms);
}

void SensorCore::readADC1(int *value_out, adc1_channel_t channel){
    int buffer;
    
    //read ADC
    adc1_get_raw(channel); //ignore first reading
    buffer = adc1_get_raw(channel);
    log_v("ADC1(%i) read at %i", channel, buffer);

    //write to provided int var
    *value_out = buffer;
}

void SensorCore::readADC2(int *value_out, adc2_channel_t channel){
    int buffer, ignore;

    //read ADC
    adc2_get_raw(channel, ADC_WIDTH_BIT_12, &ignore); //ignore first reading
    adc2_get_raw(channel, ADC_WIDTH_BIT_12, &buffer);
    log_v("ADC2(%i) read at %i", channel, buffer);

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
    log_d("Sensor %i sampled at %f", sensor, temperature);

    //debug to uart
    // char debug[128];
    // sprintf(debug, "Sensor %i -> Channel %i :: @%i -> %imV -> [%.3fR]%.3fK", sensor, adc_channel, average_reading, voltage, resistance, temperature);
    // Serial.println(debug);

    return temperature;
}

void SensorCore::snapshot(Telemetry *telemetry_out){
    //debug to uart
    // char header[64];
    // sprintf(header, "\n\n\nSnapshot at %i", realtime.getEpoch());
    // Serial.println(header);

    //Current time -> telemetry object
    unsigned long second = realtime.getEpoch();
    unsigned int milli = realtime.getMillis();
    telemetry_out->setTime(second, realtime.getMillis());

    //Read temperatues -> telemetry object
    for(int sensor = 0; sensor < number_of_temp_sensors; sensor++){
        //Gather data
        float buffer = sample(sensor);

        //Put data into Telemetry object
        telemetry_out->setTemp(sensor, buffer);
    }

    log_i("Sensor snapshot to telemetry at %i", realtime.getEpoch());
}

float SensorCore::test(){
    float average = 0;

    for(int sensor = 0; sensor < number_of_temp_sensors; sensor++){
        float buffer = sample(sensor);
        log_v("Sensor %i @ %f", sensor, buffer);
        average += buffer;
    }

    average /= number_of_temp_sensors;
    log_d("SensorCore tested. Average temperature: %f", average);

    return average;
}