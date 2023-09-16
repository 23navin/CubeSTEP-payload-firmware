/**
 * @file I2C.cpp
 * @author your name (you@domain.com)
 * 
 * @brief Implementation of I2cCore 
 * 
 */

#include "I2C.h"

I2cCore::I2cCore(){
    log_v("SDA pin: %i", sda);
    log_v("SCL pin: %i", scl);
    log_v("Device Mode: %i", I2C_MODE_SLAVE);
    log_v("Device Address: %02X", DEVICE_ADDR);

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
        log_e("I2C Parameter Config Failed");
    }
    else log_d("I2C Paramater Configured");

    err = i2c_driver_install(I2C_PORT_NUM, config.mode, BUFFER_SIZE, BUFFER_SIZE, 0);
    if (err != ESP_OK) {
        log_e("I2C Driver Install Failed");
    }
    else {
        log_i("I2C Driver Installed");
        log_d("Device Mode: %i", config.mode);
        log_d("Device Address: %02X", DEVICE_ADDR);
    }

    //If software restart by I2C (OpCode 0x21), echo parameter \
    to complete communication.
    if(esp_reset_reason() == ESP_RST_SW) {
        log_d("Software restart complete");
        write_one_byte(0x05);
    }
}

void I2cCore::reset(){
    esp_err_t err;

    err = i2c_driver_delete(I2C_PORT_NUM);
    if (err != ESP_OK) {
        log_e("I2C Driver Delete Failed");
    }
    else log_i("I2C Driver Deleted");

    init();
}

int I2cCore::read(uint8_t *rx_data){
    log_v("I2C read from buffer");
    return i2c_slave_read_buffer(0, rx_data, 1, 0);
}

int I2cCore::write(uint8_t *tx_data){
    log_v("I2C wrote to buffer");
    return i2c_slave_write_buffer(0, tx_data, 1, 0);
}

void I2cCore::write_one_byte(uint8_t tx_data){
    const int tx_size = _TX_SIZE(1);
    uint8_t *buffer = (uint8_t *)malloc(tx_size*sizeof(uint8_t)); //data to transmit
    buffer[0] = START_BYTE;
    buffer[1] = tx_data;

    log_i("Data Response: %02X", tx_data);
    i2c_slave_write_buffer(0, buffer, tx_size, 0);
}

void I2cCore::write_four_bytes(uint32_t tx_data){
    const int tx_size = _TX_SIZE(4);
    uint8_t *buffer = (uint8_t *)malloc(tx_size*sizeof(uint8_t)); //data to transmit

    buffer[0] = START_BYTE;
    buffer[1] = (tx_data & 0xFF000000) >> 24;
    buffer[2] = (tx_data & 0x00FF0000) >> 16;
    buffer[3] = (tx_data & 0x0000FF00) >> 8;
    buffer[4] = (tx_data & 0x000000FF);
    log_i("Data Response: %02X %02X %02X %02X", buffer[1], buffer[2], buffer[3], buffer[4]);
    

    i2c_slave_write_buffer(0, buffer, tx_size, 0);
}

bool I2cCore::check_for_message(){
    //check for i2c message
    if(bytes_read == 0) {
        log_v("checking i2c rx buffer");
        bytes_read += read(operation);
    }

    //if message is received but unprocessed
    if(bytes_read > 0) {
        log_i("Message Received");
        int paramater_size; //expected size of parameter argument in bytes
        parameter = 0; //reset parameter value

        //operation receipt
        bytes_read -= write(operation);

        //parse parameter length
        paramater_size = *operation >> 5;
        if(paramater_size > 4) paramater_size = 4; //parameter size should be 4 bytes or smaller
        log_d("OpCode 0x%02X", *operation);
        log_d("%d-Byte Parameter Expected", paramater_size);
        
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
            log_i("Parameter received: %i", parameter);
        }

        return true;
    }

    return false;
}

void I2cCore::install_procedure_handler(uint8_t opcode, void (*)(uint8_t)){

}