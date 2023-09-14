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

#define one_second (1000) //one thousand milli-seconds

//Logging
#define TAGI "payload_INFO"

//UART
#define UART_BAUD_RATE 115200 //default baud rate

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

void PWMtest(HeaterCore *heater_p) {
    int cycle = 0;
    while(1){
        // Telemetry test;
        // sensor.snapshot(&test);
        delay(5000);

        heater_p->setDutyCycle(cycle);

        cycle+=10;
        if(cycle > 100) cycle = 0;
    }
}

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
    Serial.printf("Communication started over UART @ %i\n", UART_BAUD_RATE);
    Serial.printf("System time is %i/%i\n\n", rtc.getEpoch(), rtc.getMillis());

    // //File testing
    // Telemetry active, capture;
    // Serial.println("file tests below");
    // storage.listDir("/", 0);
    // deleteLog(&storage);
    // createLog(&storage, &active);
    // storage.listDir("/", 0);

    // sensor.snapshot(&capture);
    // addLine(&storage, &capture);

    // active.random(&rtc);
    // addLine(&storage, &active);

    // fileTester(&storage);

    Serial.println("-----\nsetup testing done!");
    heater.initPWM();
    heater.startPWM();
    heater.setDutyCycle(40, 1);

    //i2c setup
    slave.init();
    Serial.printf("Slave joined I2C bus with addr #%02X\n-----\n", slave.getAddress());
}

void loop(){
    if(slave.check_for_message()) {
        uint8_t opcode = slave.get_opcode();

        if(opcode == 0x32) { //Get Time
            uint8_t parameter = slave.get_parameter();
            if(parameter == 0xEE) { //Epoch
                uint32_t buffer;
                buffer = rtc.getEpoch();

                slave.write_four_bytes(buffer);
            }
            else if (parameter == 0x4D) { //ms
            }
            else { //Invalid Parameter

            }
        }
        Serial.println("End Message\n");
    }
}

void get_temperature(uint8_t parameter){
    if(parameter == 0xEE) { //Epoch
        uint32_t buffer;
        buffer = rtc.getEpoch();
        slave.write_four_bytes(buffer);
        }

    else if (parameter == 0x4D) { //ms
    }
    else { //Invalid Parameter

    }
}