/**
 * @file I2C.cpp
 * @author your name (you@domain.com)
 * 
 * @brief Implementation of I2cCore 
 * 
 */

#include "I2C.h"

/*
old

string response

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
*/