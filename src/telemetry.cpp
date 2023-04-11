/**
 * @file telemetry.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of Telemetry structure
**/

#include "Telemetry.h"

Telemetry::Telemetry() {
    clear();
}

void Telemetry::toCSV(char *tempChar, int channel) {
    char buffer[char_length_temp];
    if(SENS[channel] < 1) {
        strcpy(tempChar, "XXX.XXX");
    }
    else {
        int ret = snprintf(buffer, char_length_temp, "%7.3f", SENS[channel]);
        if(ret > 0) {
            strcpy(tempChar, buffer);
        }
    }
}

void Telemetry::tocSV(char *wattChar, int channel) {
        char buffer[char_length_watt];
    if(SENS[channel] < 1) {
        strcpy(wattChar, "XX.XXX");
    }
    else {
        int ret = snprintf(buffer, char_length_watt, "%6.3f", Watt[channel]);
        if(ret > 0) {
            strcpy(wattChar, buffer);
        }
    }
}

void Telemetry::clear() {
    for(int i = 0; i < number_of_temp_sensors; i++) {
        SENS[i] = 0.0;
    }

    for(int i = 0; i < number_of_watt_outs; i++) {
        Watt[i] = 0.0;
    }

    seconds = 0; //change to set current time
    useconds = 0;
}