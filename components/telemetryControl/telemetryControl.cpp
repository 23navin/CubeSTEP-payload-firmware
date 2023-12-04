/**
 * @file Telemetry.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of Telemetry structure
**/

#include "telemetryControl.h"


unsigned long epoch(){ //get time for srand seed
    time_t now;
    time(&now);
    return now;
}

telemetryControl::Telemetry::Telemetry(){
    clear(); //sets to default values
    srand(epoch()); //random seed for testing
}

void telemetryControl::Telemetry::ToCSV(char *LineChar){
    char Line[sizeLine]; // "<time>,<pwm>,<temp>\n\0"
    char TimeBuffer[sizeTime]; // "<sec>,<usec>\0"
    char PWMBuffer[sizePWM]; // "<pwm0>,<pwm1>,...\0"
    char TempBuffer[sizeTemp]; // "<sens0>,<sens1>,...\0"

    //time
    TimeToCSV(TimeBuffer);
    strcpy(Line, TimeBuffer);

    strncat(Line, ",", 2);

    //PWM
    PWMToCSV(PWMBuffer);
    strncat(Line, PWMBuffer, sizePWM);

    strncat(Line, ",", 2);

    //temp
    TempToCSV(TempBuffer);
    strncat(Line, TempBuffer, sizeTemp);

    //end
    strncat(Line,"\n",2); // end line
    strcpy(LineChar, Line);
}

void telemetryControl::Telemetry::TimeToCSV(char *TimeChar){
    char Buffer[sizeTime]; // "<sec>,<usec>\0"
    char SecBuffer[sizeEpoch]; // "<sec>\0"
    char uSecBuffer[sizeMicroSecond]; // "<usec>\0"

    TimeToCSV(SecBuffer, uSecBuffer);

    snprintf(Buffer, sizeTime, "%s,%s",SecBuffer, uSecBuffer);
    strcpy(TimeChar, Buffer);
}

void telemetryControl::Telemetry::TimeToCSV(char *SecondsChar, char*uSecondsChar){
    char Buffer[sizeEpoch]; // "<sec>\0"
    char uBuffer[sizeMicroSecond]; // <usec>\0
  
    int ret = snprintf(Buffer, sizeEpoch, "%.10lu", Seconds); // todo: add definitions
    if(ret > 0){
        strcpy(SecondsChar, Buffer);
    }

    ret = snprintf(uBuffer, sizeMicroSecond, "%.6lu", uSeconds); // todo: add definitions
    if(ret > 0){
        strcpy(uSecondsChar, uBuffer);
    }
}

void telemetryControl::Telemetry::TempToCSV(char *TempChar){
    char Buffer[sizeTemp]; // "<sens0>,<sens1>,...\0"

    volatile int dst_size = sizeSensor; //workaround for gcc format-truncation warning
    snprintf(Buffer, dst_size, "%7f", Sens[0]);

    for(int i=1; i<numSensors; i++){
        char SensBuffer[sizeSensor];
        char CellBuffer[sizeSensor+1];
        
        TempToCSV(SensBuffer, i);
        snprintf(CellBuffer, sizeSensor+1, ",%s",SensBuffer);
        strncat(Buffer, CellBuffer, sizeSensor+1);
    }

    strcpy(TempChar, Buffer);
}

void telemetryControl::Telemetry::TempToCSV(char *CellChar, int channel){
    char Buffer[sizeSensor];
    
    int ret = snprintf(Buffer, sizeSensor, "%7f", Sens[channel]); // todo: add definitions

    if(ret > 0){
        strcpy(CellChar, Buffer);
    }
}

void telemetryControl::Telemetry::PWMToCSV(char *PWMChar){
    char Buffer[sizePWM]; // "<duty>,<period>\0"
    char DutyBuffer[sizePWMDuty]; // "<duty>\0"
    char PeriodBuffer[sizePWMPeriod]; // "<period>\0"

    PWMToCSV(DutyBuffer, PeriodBuffer);

    snprintf(Buffer, sizePWM, "%s,%s",DutyBuffer, PeriodBuffer);
    strcpy(PWMChar, Buffer);
}

void telemetryControl::Telemetry::PWMToCSV(char *DutyChar, char*PeriodChar){
    char DutyBuffer[sizeEpoch]; // "<duty>\0"
    char PeriodBuffer[sizeMicroSecond]; // <period>\0

    int ret = snprintf(DutyBuffer, sizePWMDuty, "%3i", pwm_Duty); // todo: add definitions
    if(ret > 0){
        strcpy(DutyChar, DutyBuffer);
    }

    ret = snprintf(PeriodBuffer, sizePWMPeriod, "%4f", pwm_Period); // todo: add definitions
    if(ret > 0){
        strcpy(PeriodChar, PeriodBuffer);
    }
}

void telemetryControl::Telemetry::headerCSV(char *LineChar){
    char LineBuffer[1000];

    //time
    strcpy(LineBuffer, "sys_Time(S),sys_Time(uS)");
    
    //pwm
    strcat(LineBuffer, ",pwm_Duty(%),pwm_Period(S)");

    //temp
    for(int i=0; i<numSensors; i++){
        char buffer[100];
        sprintf(buffer, ",temp_%d(K)", i);
        strcat(LineBuffer, buffer);
    }

    strncat(LineBuffer,"\n",2); // end line
    //out
    strcpy(LineChar, LineBuffer);
}

void telemetryControl::Telemetry::clear(){
    Seconds = 0;
    uSeconds = 0;

    pwm_Duty = 0;
    pwm_Period = 0;

    for(int i = 0; i < numSensors; i++){
        Sens[i] = 0.0;
    }
}

void telemetryControl::Telemetry::setTemp(int sensor, float temperature){
    Sens[sensor] = temperature;
}

void telemetryControl::Telemetry::setTemp(float *temp_array[]){
    for(int i = 0; i < numSensors; i++){
        float buffer = *temp_array[i];
        Sens[i] = buffer;
    }
}

void telemetryControl::Telemetry::setTime(){
    struct timeval tv;
    gettimeofday(&tv, NULL);

    Seconds = tv.tv_sec;
    uSeconds = tv.tv_usec;
}

void telemetryControl::Telemetry::setTime(unsigned long epoch, unsigned long micro){
    Seconds = epoch;
    uSeconds = micro;
}

void telemetryControl::Telemetry::setPWM(int duty_percentage, float cycle_period){
    pwm_Duty = duty_percentage;
    pwm_Period = cycle_period;
}
