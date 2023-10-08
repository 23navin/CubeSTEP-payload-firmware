/**
 * @file Telemetry.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of Telemetry structure
**/

#include "Telemetry.h"

unsigned long epoch(){ //get time for srand seed
    time_t now;
    time(&now);
    return now;
}

Telemetry::Telemetry(){
    clear(); //sets to default values
    srand(epoch()); //random seed for testing
}

void Telemetry::ToCSV(char *LineChar){
    char Line[line_size]; // "<time>,<pwm>,<temp>\n\0"
    char TimeBuffer[type_size_time]; // "<sec>,<usec>\0"
    char PWMBuffer[type_size_pwm]; // "<pwm0>,<pwm1>,...\0"
    char TempBuffer[type_size_temp]; // "<sens0>,<sens1>,...\0"

    //time
    TimeToCSV(TimeBuffer);
    strcpy(Line, TimeBuffer);

    strncat(Line, ",", 2);

    //PWM
    PWMToCSV(PWMBuffer);
    strncat(Line, PWMBuffer, type_size_pwm);

    strncat(Line, ",", 2);

    //temp
    TempToCSV(TempBuffer);
    strncat(Line, TempBuffer, type_size_temp);

    //end
    strncat(Line,"\n",2); // end line
    strcpy(LineChar, Line);
}

void Telemetry::TimeToCSV(char *TimeChar){
    char Buffer[type_size_time]; // "<sec>,<usec>\0"
    char SecBuffer[cell_size_epoch]; // "<sec>\0"
    char uSecBuffer[cell_size_mSecond]; // "<usec>\0"

    TimeToCSV(SecBuffer, uSecBuffer);

    snprintf(Buffer, type_size_time, "%s,%s",SecBuffer, uSecBuffer);
    strcpy(TimeChar, Buffer);
}

void Telemetry::TimeToCSV(char *SecondsChar, char*uSecondsChar){
    char Buffer[cell_size_epoch]; // "<sec>\0"
    char uBuffer[cell_size_mSecond]; // <usec>\0

    if((SecondsChar < 0) && (uSecondsChar < 0)){ //need to increase robustness/error checking
        strcpy(SecondsChar, "XXXXXXXX");
        strcpy(uSecondsChar, "XXX");
    }
    else {
        int ret = snprintf(Buffer, cell_size_epoch, "%.10i", Seconds); // todo: add definitions
        if(ret > 0){
            strcpy(SecondsChar, Buffer);
        }

        ret = snprintf(uBuffer, cell_size_mSecond, "%.3i", mSeconds); // todo: add definitions
        if(ret > 0){
            strcpy(uSecondsChar, uBuffer);
        }
    }
}

void Telemetry::TempToCSV(char *TempChar){
    char Buffer[type_size_temp]; // "<sens0>,<sens1>,...\0"

    snprintf(Buffer, cell_size_temp, "%7f", Sens[0]);

    for(int i=1; i<number_of_temp_sensors; i++){
        char SensBuffer[cell_size_temp];
        char CellBuffer[cell_size_temp+1];
        
        TempToCSV(SensBuffer, i);
        snprintf(CellBuffer, cell_size_temp+1, ",%s",SensBuffer);
        strncat(Buffer, CellBuffer, cell_size_temp+1);
    }

    strcpy(TempChar, Buffer);
}

void Telemetry::TempToCSV(char *CellChar, int channel){
    char Buffer[cell_size_temp];
    
    int ret = snprintf(Buffer, cell_size_temp, "%7f", Sens[channel]); // todo: add definitions

    if(ret > 0){
        strcpy(CellChar, Buffer);
    }
}

// todo: add option to use XXX.XXX if < 0 using preprocessor

// void Telemetry::TempToCSV(char *TempChar){
//     char Buffer[type_size_temp]; // "<sens0>,<sens1>,...\0"
//     if(Sens[0] < 1.0){
//         strcpy(Buffer, "XXX.XXX"); // todo: need to make function with respect to definitions
//     }
//     else {
//         snprintf(Buffer, cell_size_temp, "%7.3f", Sens[0]);
//     }

//     for(int i=1; i<number_of_temp_sensors; i++){
//         char SensBuffer[cell_size_temp];
//         char CellBuffer[cell_size_temp+1];
//         if(Sens[i] < 1.0){
//             strncat(Buffer, ",XXX.XXX", cell_size_temp+1);
//         }
//         else {
//             TempToCSV(SensBuffer, i);
//             snprintf(CellBuffer, cell_size_temp+1, ",%s",SensBuffer);
//             strncat(Buffer, CellBuffer, cell_size_temp+1);
//         }
//     }

//     strcpy(TempChar, Buffer);
// }

// void Telemetry::TempToCSV(char *CellChar, int channel){
//     char Buffer[cell_size_temp];
    
//     if(Sens[channel] < 1.0){
//         strcpy(CellChar, "XXX.XXX"); // todo: need to make function with respect to definitions
//     }
//     else {
//         int ret = snprintf(Buffer, cell_size_temp, "%7.3f", Sens[channel]); // todo: add definitions

//         if(ret > 0){
//             strcpy(CellChar, Buffer);
//         }
//     }
// }

void Telemetry::PWMToCSV(char *PWMChar){
    char Buffer[type_size_pwm]; // "<duty>,<period>\0"
    char DutyBuffer[cell_size_pwm_duty]; // "<duty>\0"
    char PeriodBuffer[cell_size_pwm_period]; // "<period>\0"

    PWMToCSV(DutyBuffer, PeriodBuffer);

    snprintf(Buffer, type_size_pwm, "%s,%s",DutyBuffer, PeriodBuffer);
    strcpy(PWMChar, Buffer);
}

void Telemetry::PWMToCSV(char *DutyChar, char*PeriodChar){
    char DutyBuffer[cell_size_epoch]; // "<duty>\0"
    char PeriodBuffer[cell_size_mSecond]; // <period>\0

    int ret = snprintf(DutyBuffer, cell_size_pwm_duty, "%3i", pwm_Duty); // todo: add definitions
    if(ret > 0){
        strcpy(DutyChar, DutyBuffer);
    }

    ret = snprintf(PeriodBuffer, cell_size_pwm_period, "%4f", pwm_Period); // todo: add definitions
    if(ret > 0){
        strcpy(PeriodChar, PeriodBuffer);
    }
}

void Telemetry::headerCSV(char *LineChar){
    char LineBuffer[1000];

    //time
    strcpy(LineBuffer, "sys_Time(S),sys_Time(mS)");
    
    //pwm
    strcat(LineBuffer, ",pwm_Duty(%),pwm_Period(S)");

    //temp
    for(int i=0; i<number_of_temp_sensors; i++){
        char buffer[100];
        sprintf(buffer, ",temp_%d(K)", i);
        strcat(LineBuffer, buffer);
    }

    strncat(LineBuffer,"\n",2); // end line
    //out
    strcpy(LineChar, LineBuffer);
}

void Telemetry::clear(){
    Seconds = 0;
    mSeconds = 0;

    pwm_Duty = 0;
    pwm_Period = 0;

    for(int i = 0; i < number_of_temp_sensors; i++){
        Sens[i] = 0.0;
    }
}

//do not use
void Telemetry::random(ESP32Time *realtime){
    delay(rand() % 5000);

    //time
    Seconds = realtime->getEpoch();
    mSeconds = realtime->getMillis();

    //pwm
    pwm_Duty = rand() % 101 + 1;
    pwm_Period = 12;

    //temp
    for(int i=0; i < number_of_temp_sensors; i++){
        Sens[i] = 1.0 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(500.0-1.0)));
    }
}

void Telemetry::setTemp(int sensor, float temperature){
    Sens[sensor] = temperature;
}

void Telemetry::setTemp(float *temp_array[]){
    for(int i = 0; i < number_of_temp_sensors; i++){
        float buffer = *temp_array[i];
        Sens[i] = buffer;
    }
}

void Telemetry::setTime(){
    Seconds = realtime.getEpoch();
    mSeconds = realtime.getMillis();
}

void Telemetry::setTime(unsigned long epoch, unsigned int milli){
    Seconds = epoch;
    mSeconds = milli;
}

void Telemetry::setPWM(int duty_percentage, float cycle_period){
    pwm_Duty = duty_percentage;
    pwm_Period = cycle_period;
}
