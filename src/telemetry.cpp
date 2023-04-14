/**
 * @file telemetry.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of Telemetry structure
**/

#include "Telemetry.h"

Telemetry::Telemetry() {
    clear(); //sets to default values
}

void Telemetry::ToCSV(char *LineChar) {
    char Line[line_size]; // "<time>,<watt>,<temp>\n\0"

    //time
    char TimeBuffer[type_size_time]; // "<sec>,<usec>\0"

    TimeToCSV(TimeBuffer);
    strcpy(Line, TimeBuffer);

    strncat(Line, ",", 2);
    //temp
    char TempBuffer[type_size_temp]; // "<sens0>,<sens1>,...\0"

    TempToCSV(TempBuffer);
    strncat(Line, TempBuffer, type_size_temp);

    strncat(Line, ",", 2);
    //Watt
    char WattBuffer[type_size_watt]; // "<watt0>,<watt1>,...\0"

    WattToCSV(WattBuffer);
    strncat(Line, WattBuffer, type_size_watt);

    strncat(Line,"\n",2); // end line
    //output
    strcpy(LineChar, Line);
}

void Telemetry::TimeToCSV(char *TimeChar) {
    char Buffer[type_size_time]; // "<sec>,<usec>\0"
    char SecBuffer[cell_size_time]; // "<sec>\0"
    char uSecBuffer[cell_size_time]; // "<usec>\0"

    TimeToCSV(SecBuffer, uSecBuffer);

    snprintf(Buffer, type_size_time, "%s,%s",SecBuffer, uSecBuffer);
    strcpy(TimeChar, Buffer);
}

void Telemetry::TimeToCSV(char *SecondsChar, char*uSecondsChar) {
    char Buffer[cell_size_time]; // "<sec>\0"
    char uBuffer[cell_size_time]; // <usec>\0

    if((SecondsChar < 0) && (uSecondsChar < 0)) { //need to increase robustness/error checking
        strcpy(SecondsChar, "XXXXXXXX");
        strcpy(uSecondsChar, "XXXXXXXX");
    }
    else {
        int ret = snprintf(Buffer, cell_size_time, "%.8x", Seconds); // todo: add definitions
        if(ret > 0) {
            strcpy(SecondsChar, Buffer);
        }

        ret = snprintf(uBuffer, cell_size_time, "%.8x", uSeconds); // todo: add definitions
        if(ret > 0) {
            strcpy(uSecondsChar, uBuffer);
        }
    }
}

void Telemetry::TempToCSV(char *TempChar) {
    char Buffer[type_size_temp]; // "<sens0>,<sens1>,...\0"
    if(Sens[0] < 1.0) {
        strcpy(Buffer, "XXX.XXX"); // todo: need to make function with respect to definitions
    }
    else {
        snprintf(Buffer, cell_size_temp, "%7.3f", Sens[0]);
    }

    for(int i=1; i<number_of_temp_sensors; i++) {
        char SensBuffer[cell_size_temp];
        char CellBuffer[cell_size_temp+1];
        if(Sens[i] < 1.0) {
            strncat(Buffer, ",XXX.XXX", cell_size_temp+1);
        }
        else {
            TempToCSV(SensBuffer, i);
            snprintf(CellBuffer, cell_size_temp+1, ",%s",SensBuffer);
            strncat(Buffer, CellBuffer, cell_size_temp+1);
        }
    }

    strcpy(TempChar, Buffer);
}

void Telemetry::TempToCSV(char *CellChar, int channel) {
    char Buffer[cell_size_temp];
    
    if(Sens[channel] < 1.0) {
        strcpy(CellChar, "XXX.XXX"); // todo: need to make function with respect to definitions
    }
    else {
        int ret = snprintf(Buffer, cell_size_temp, "%7.3f", Sens[channel]); // todo: add definitions

        if(ret > 0) {
            strcpy(CellChar, Buffer);
        }
    }
}

void Telemetry::WattToCSV(char *WattChar) {
    char Buffer[type_size_watt];
    if(Watt[0] < 0.0) {
        strcpy(Buffer, "XX.XXX"); // todo: need to make function with respect to definitions
    }
    else {
        snprintf(Buffer, cell_size_watt, "%6.3f", Watt[0]);
    }

    for(int i=1; i<number_of_watt_outs; i++) {
        char WattBuffer[cell_size_watt];
        char CellBuffer[cell_size_watt+1];
        if(Watt[i] < 0.0) {
            strncat(Buffer, ",XX.XXX", cell_size_watt+1);
        }
        else {
            WattToCSV(WattBuffer, i);
            snprintf(CellBuffer, cell_size_watt+1, ",%s",WattBuffer);
            strncat(Buffer, CellBuffer, cell_size_watt+1);
        }
    }

    strcpy(WattChar, Buffer);
}

void Telemetry::WattToCSV(char *CellChar, int channel) {
    char Buffer[cell_size_watt];

    if(Watt[channel] < 0) {
        strcpy(CellChar, "XX.XXX");
    }
    else {
        int ret = snprintf(Buffer, cell_size_watt, "%6.3f", Watt[channel]); // todo: add definitions
        if(ret > 0) {
            strcpy(CellChar, Buffer);
        }
    }
}

void Telemetry::headerCSV(char *LineChar) {
    char LineBuffer[1000];

    //time
    strcpy(LineBuffer, "system_time (seconds),system_time (micro_seconds)");
    //temp
    for(int i=0; i<number_of_temp_sensors; i++) {
        char buffer[100];
        sprintf(buffer, ",temp_sensor_%d (degrees_kelvin)", i);
        strcat(LineBuffer, buffer);
    }

    //watt
    for(int i=0; i<number_of_watt_outs; i++) {
        char buffer[100];
        sprintf(buffer, ",watt_out_%d (volts)", i);
        strcat(LineBuffer, buffer);
    }

    strncat(LineBuffer,"\n",2); // end line
    //out
    strcpy(LineChar, LineBuffer);
}

void Telemetry::clear() {
    for(int i = 0; i < number_of_temp_sensors; i++) {
        Sens[i] = 0.0;
    }

    for(int i = 0; i < number_of_watt_outs; i++) {
        Watt[i] = 0.0;
    }

    Seconds = 0; // todo: change to set current time
    uSeconds = 0;
}

void Telemetry::random() {
    //time
    Seconds = (rand() % 0x100000000);
    uSeconds = (rand() % 0x100000000);

    //temp
    for(int i=0; i < number_of_temp_sensors; i++) {
        Sens[i] = 1.0 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(500.0-1.0)));
    }

    //watt
    for(int i=0; i < number_of_watt_outs; i++) {
        Watt[i] = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(12.0)));
    }
}