/**
 * @file main.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief ESP32 Payload Processor firmware
 * //will add documentation later 
 */


using namespace std;
#include <Arduino.h>
#include <Wire.h>

#include "File.h"
#include "Telemetry.h"
#include "Experiment.h"
#include "I2C.h"
#include "Sensor.h"

// I2C bus
#define SDA 1               // SDA pin
#define SCL 2               // SCL pin
#define SLAVE_ADDRESS 0x04  // device address
#define ANSWER_SIZE 5       // length of message that is sent over I2C

// ADC pins
#define SENS1 1     // temperature sensor from ADC channel 1
#define SENS2 2     // temperature sensor from ADC channel 2
#define SENS3 3     // temperature sensor from ADC channel 3
#define SENS4 4     // temperature sensor from ADC channel 4
#define SENS5 5     // temperature sensor from ADC channel 5
#define SENS6 6     // temperature sensor from ADC channel 6
#define SENS7 7     // temperature sensor from ADC channel 7
#define SENS8 8     // temperature sensor from ADC channel 8
#define SENS9 9     // temperature sensor from ADC channel 9
#define SENS10 10   // temperature sensor from ADC channel 10
#define SENS11 11   // temperature sensor from ADC channel 11
#define SENS12 12   // temperature sensor from ADC channel 12
#define SENS13 13   // temperature sensor from ADC channel 13
#define SENS14 14   // temperature sensor from ADC channel 14
#define SENS15 15   // temperature sensor from ADC channel 15
#define SENS16 16   // temperature sensor from ADC channel 16

// ADC -> Temperature
#define THERMISTORNOMINAL 10000     // (▀̿ ̿̀ ﹏ ́▀̿ ̿ )
#define TEMPERATURENOMINAL 25       // (≖̀ ﹏ ́≖)
#define NUMSAMPLES 5                // number of samples to be averages for each adc output
#define BCOEFFICIENT 3950           // god knows what this is
#define SERIESRESISTOR 10000        // cause i sure as fuck dont

const byte SENSOR[] = {SENS1, SENS2, SENS3, SENS4, SENS5, SENS6, SENS7, SENS8, SENS9, SENS10, SENS11, SENS12, SENS13, SENS14, SENS15, SENS16};

String response;

// I2C bus
void ReceiveI2C(int length) {
    while(0 < Wire.available()) {
        byte x = Wire.read();
    }

    Serial.println("I2C Received!");
}

void WritetoI2C() {
    byte message[ANSWER_SIZE];

    for(byte i=0; i<ANSWER_SIZE; i++) {
        message[i] = (byte)response.charAt(i);
    }

    Wire.write(message, sizeof(message));

    Serial.println("I2C Sent!");
}

// Record Temps
float ReadADC(byte pin){
    float reading = analogRead(pin);
    if(reading < 1 || reading > 4095) return 0;
    return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
}

void checkSENS(float* tempArray[16]) {
    //sample 16 sensors
    for(int i=0; i<16; i++) {
        double temp = 0;

        //sample ADC
        for(int j=0; j<NUMSAMPLES; j++) {
            temp += ReadADC(SENSOR[j]);
        }
        temp /= NUMSAMPLES; // averaged sample
        temp = 1023 / temp - 1;       // sample -> resistance
        temp = SERIESRESISTOR / temp; //
        temp = temp / THERMISTORNOMINAL; // R/Ro
        temp = log(temp); // ln(R/Ro)
        temp /= BCOEFFICIENT; // 1/B * ln(R/Ro)
        temp += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
        temp = 1.0 / temp; // Invert
        temp -= 273.15; // Convert

        *tempArray[i]=temp; //send to array
    }

    Serial.println("Tempuratures read");
}

void createLog(FileCore *file_p, Telemetry *tele){
    char header[1000];
    tele->headerCSV(header);
    file_p->writeFile("/log.csv", header);
}

void addLine(FileCore *file_p, Telemetry *tele) {
    char line[line_size];
    tele->ToCSV(line);
    file_p->appendFile("/log.csv", line);
    
}

void initSPI(FileCore *file_p) {
    file_p->mount();
    //createLog();
}

void teleTester(Telemetry *tele, FileCore *file_p) {
    tele->random();

    char header[1000];
    char tempout[type_size_temp];
    char wattout[type_size_watt];
    char secondOut[cell_size_time];
    char usecondOut[cell_size_time];
    char timeOut[type_size_time];
    char Out[line_size];

    tele->TempToCSV(tempout);
    tele->WattToCSV(wattout);
    tele->TimeToCSV(secondOut, usecondOut);
    tele->TimeToCSV(timeOut);
    tele->headerCSV(header);
    tele->ToCSV(Out);

    
    Serial.println(tempout);
    Serial.println(wattout);
    Serial.println(secondOut);
    Serial.println(usecondOut);
    Serial.println(timeOut);
    Serial.println(header);
    Serial.println(Out);
}

void fileTester(FileCore *file_p) {
    vector<String> recovered;
    recovered = file_p->loadFile("/log.csv");
    
    Serial.println(recovered.size());
    Serial.println(recovered[0]);
}

FileCore storage;
ExperimentCore experiment;
I2cCore slave;
SensorCore sensor;

void setup() {
    // Serial setup
    Serial.begin(115200);
    delay(1000);
    Serial.println("Communication started\n");

    // Telemetry testing
    Serial.println("telemetry tests below");

    Telemetry active;
    teleTester(&active, &storage);

    //File testing
    Serial.println("file tests below");
    storage.mount();
    storage.listDir("/", 0);

    //createLog(&file, &active);
    //file.listDir("/",0);
    //addLine(&storage, &active);
    fileTester(&storage);


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
}

void loop() {
  // put your main code here, to run repeatedly:
}