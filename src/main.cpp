#include <Arduino.h>
#include <Wire.h>
#include "SPI.h"
using namespace std;

// I2C bus
#define SDA 1
#define SCL 2
#define SLAVE_ADDRESS 0x04
#define ANSWER_SIZE 5

// ADC pins
#define SENS1 1
#define SENS2 2
#define SENS3 3
#define SENS4 4
#define SENS5 5
#define SENS6 6
#define SENS7 7
#define SENS8 8
#define SENS9 9
#define SENS10 10
#define SENS11 11
#define SENS12 12
#define SENS13 13
#define SENS14 14
#define SENS15 15
#define SENS16 16

// ADC -> Temperature
#define THERMISTORNOMINAL 10000
#define TEMPERATURENOMINAL 25
#define NUMSAMPLES 5
#define BCOEFFICIENT 3950
#define SERIESRESISTOR 10000

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

void testSENS(float* tempArray[16]) {
    

}

void FillChar(char output[], int input) {
    itoa(input, output, 16);
}

void createLog(FileCore *file_p){
    file_p->writeFile(SPIFFS, "/log.csv", "TimeL, TimeS, PWM, SENS0, SENS1\n");
}

void packMessage(char* messageOut, char time[2], char temps[16], char wattage[2]) {
    char data[20];

    data[0] = time[0];
    data[1] = time[1];

    data[18] = wattage[0];
    data[19] = wattage[1];

    for(int i = 0; i < 16; i++) {
        data[i+2] = temps[i];
    }

    messageOut = data;
}

void addLine(FileCore *file_p, char time[2], char temps[16], char wattage[2]) {
    char message[20];
    
    packMessage(message, time, temps, wattage);

    file_p->appendFile(SPIFFS, "/log.csv", message);
    
}

void initSPI(FileCore *file_p) {
    file_p->mount();
    //createLog();
}

FileCore file;

void setup() {
    // Serial setup
    Serial.begin(115200);
    Serial.println("Communication started");
    Serial.println("starting!\n");

    // SPI setup
    initSPI(&file);

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
}

void loop() {
  // put your main code here, to run repeatedly:
}