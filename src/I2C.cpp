/**
 * @file I2C.cpp
 * @author your name (you@domain.com)
 * 
 * @brief Implementation of I2cCore 
 * 
 */

#include "I2C.h"

I2cCore::I2cCore(){
    config.sda_io_num = sda;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_io_num = scl;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.mode = I2C_MODE_SLAVE;
    config.slave.addr_10bit_en = 0;
    config.slave.slave_addr = address & 0x7F;
    config.slave.maximum_speed = 400000UL;
    config.clk_flags = 0;
}

I2cCore::~I2cCore(){
    i2c_driver_delete(I2C_PORT_NUM);
}

void I2cCore::init(){
    esp_err_t err;

    err = i2c_param_config(I2C_PORT_NUM, &config);
    if (err != ESP_OK) {
        log_e("I2C Parameter Config Failure");
    }

    i2c_driver_install(I2C_PORT_NUM, config.mode, BUFFER_SIZE, BUFFER_SIZE, 0);
    if (err != ESP_OK) {
        log_e("I2C Driver Install Failure");
    }
}

int I2cCore::read(uint8_t *rx_data){
    return i2c_slave_read_buffer(0, rx_data, 1, 0);
}

int I2cCore::write(uint8_t *tx_data){
    return i2c_slave_write_buffer(0, tx_data, 1, 0);
}

void I2cCore::write_four_bytes(uint32_t tx_data){
    uint8_t *buffer = (uint8_t *)malloc(TX_SIZE*sizeof(uint8_t)); //data to transmit

    buffer[0] = START_BYTE;
    buffer[1] = (tx_data & 0xFF000000) >> 24;
    buffer[2] = (tx_data & 0x00FF0000) >> 16;
    buffer[3] = (tx_data & 0x0000FF00) >> 8;
    buffer[4] = (tx_data & 0x000000FF);
    
    i2c_slave_write_buffer(0, buffer, TX_SIZE, 0);
}

bool I2cCore::check_for_message(){
    //check for i2c message
    if(bytes_read == 0) {
        bytes_read += read(operation);
    }

    //if message is received but unprocessed
    if(bytes_read > 0) {
        int paramater_size; //expected size of parameter argument in bytes
        parameter = 0; //reset parameter value

        //operation receipt
        bytes_read -= write(operation);

        //parse parameter length
        paramater_size = *operation >> 5;
        if(paramater_size > 4) paramater_size = 4; //parameter size should be 4 bytes or smaller
        Serial.printf("Message received 0x%02X -> %d-Byte Parameter\n", *operation, paramater_size);
        
        //process parameter if applicable
        if(paramater_size > 0) {
            uint8_t *rx_data = (uint8_t *)malloc(sizeof(uint8_t)); //byte received over i2c

            //receive parameter byte(s)
            while(paramater_size > 0) {
                //check for i2c message
                if(read(rx_data)) {
                    paramater_size--;

                    //combine bytes
                    parameter += *rx_data << (8*paramater_size);
                }
            }
            Serial.printf("Parameter received: 0x%X\n", parameter);
        }

        return true;
    }

    return false;
}

void I2cCore::install_procedure_handler(uint8_t opcode, void (*)(uint8_t)){
    
}