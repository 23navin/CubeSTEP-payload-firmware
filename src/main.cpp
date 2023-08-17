/**
 * @file main.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief ESP32 Payload Processor firmware
 * //will add documentation later 
*/

using namespace std;
#include "ESP32Time.h"
#include "esp_log.h"
#include "esp_err.h"

#include "File.h"
#include "Telemetry.h"
#include "Sensor.h"
#include "Heater.h"
#include "I2C.h"

#define on 1 // True
#define off 0 // False
#define one_second (1000) //one thousand milli-seconds

//Logging
#define TAGI "payload_INFO"

// I2C bus -> move into i2c.h
#define SDA 1               // SDA pin
#define SCL 2               // SCL pin
#define SLAVE_ADDRESS 0x04  // device address
#define ANSWER_SIZE 5       // length of message that is sent over I2C

//UART
#define UART_BAUD_RATE 115200 //default baud rate

// FSM setup
enum class modes {
    Safety,
    LowPower,
    Idle,
    Experiment,
    Communicate
};

/**
 * @brief Sets a time object to the system time when this build was compiled
 * @note Useful as a baseline time variable
 * 
 * @param object ESP32Time pointer
 */
void setTimeToCompile(ESP32Time *object)
{
    char s_month[5];
    int year;
    struct tm t;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(__DATE__, "%s %d %d", s_month, &t.tm_mday, &year);
    sscanf(__TIME__, "%2d %*c %2d %*c %2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
    t.tm_mon = (strstr(month_names, s_month) - month_names) / 3;    
    t.tm_year = year - 1900;   

    object->setTime(mktime(&t));
}

/**
 * @brief Creates a Log object
 * @note for testing
 * 
 * @param file_p 
 * @param tele 
 */
void createLog(FileCore *file_p, Telemetry *tele){
    char header[1000];
    tele->headerCSV(header);
    file_p->writeFile("/log.csv", header);
}

void deleteLog(FileCore *file_p) {
    file_p->deleteFile("/log.csv");
}

/**
 * @brief Adds line to file
 * @note for testing
 * 
 * @param file_p 
 * @param tele 
 */
void addLine(FileCore *file_p, Telemetry *tele){
    char line[line_size];
    tele->ToCSV(line);
    file_p->appendFile("/log.csv", line);
    
}

/**
 * @brief Initializes file system
 * @note for testing
 * 
 * @param file_p 
 */
void initSPI(FileCore *file_p){
    file_p->mount();
    //createLog();
}

// void teleTester(Telemetry *tele, FileCore *file_p){
//     tele->random();
//     char header[1000];
//     char tempout[type_size_temp];
//     char wattout[type_size_watt];
//     char secondOut[cell_size_epoch];
//     char msecondOut[cell_size_mSecond];
//     char timeOut[type_size_time];
//     char Out[line_size];

//     tele->TempToCSV(tempout);
//     tele->WattToCSV(wattout);
//     tele->TimeToCSV(secondOut, msecondOut);
//     tele->TimeToCSV(timeOut);
//     tele->headerCSV(header);
//     tele->ToCSV(Out);

    
//     Serial.println(tempout);
//     Serial.println(wattout);
//     Serial.println(secondOut);
//     Serial.println(msecondOut);
//     Serial.println(timeOut);
//     Serial.println(header);
//     Serial.println(Out);
// }

void fileTester(FileCore *file_p){
    vector<String> recovered;
    recovered = file_p->loadFile("/log.csv");
    
    Serial.println(recovered.size());
    for(int i=0; i < recovered.size(); i++) {
        char line_buffer[line_size] ;
        sprintf(line_buffer, "Line #%d", i);
        Serial.println(line_buffer);
        Serial.println(recovered[i]);
    }
}

void internalSystemCheck();

void checkI2C();

void checkTime();

ESP32Time rtc;
FileCore storage;
SensorCore sensor;
HeaterCore heater;
I2cCore slave;

void setup(){
    //set internal rtc clock
    setTimeToCompile(&rtc);
    sensor.init(&rtc);

    // Serial setup
    Serial.begin(UART_BAUD_RATE);
    delay(1000);
    char jog[64];
    sprintf(jog, "Communication started over UART @ %i", UART_BAUD_RATE);
    Serial.println("Communication started");

    
    //Timestamp
    char timestamp[64];
    sprintf(timestamp, "System time is %i/%i\n", rtc.getEpoch(), rtc.getMillis());
    Serial.println(timestamp);
    
    // Telemetry testing
    // Serial.println("telemetry tests below");

    // Telemetry active, capture;

    // //File testing
    // Serial.println("file tests below");
    // storage.listDir("/", 0);
    // createLog(&storage, &active);
    // storage.listDir("/", 0);

    // sensor.snapshot(&capture);
    // addLine(&storage, &capture);

    // active.random(&rtc);
    // addLine(&storage, &active);

    // fileTester(&storage);

    // storage.listDir("/", 0);
    // deleteLog(&storage);
    // storage.listDir("/", 0);


    // I2C slave setup
    /*
    Wire.begin(SDA, SCL, SLAVE_ADDRESS);
    Wire.onReceive(ReceiveI2C);
    Wire.onRequest(WritetoI2C);
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return;
    }
    */

    Serial.println("-----\nsetup testing done!");
    heater.initPWM();
    heater.startPWM();
    // heater.setPWM(0.1, (float)2);
    // heater.startPWM();

}

void loop(){
    heater.setPWM(2,1);

    float getCycle = heater.getCyclePeriod();
    float getDuty = heater.getDutyPeriod();

    float delta = getCycle/20; //0.1
    float dutyS = getDuty + delta;

    while(on){
        //Telemetry test;
        //sensor.snapshot(&test);

        if(heater.pwmAction()){
            Serial.println("Alarm!");
            heater.setDuty(heater.getDutyPeriod()+delta);
            if(heater.getDutyPeriod() >= heater.getCyclePeriod()) heater.setDuty(delta);
        }
    }
}