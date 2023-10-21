/**
 * @file I2C.cpp
 * @author your name (you@domain.com)
 * 
 * @brief Implementation of i2c 
 * 
 */

#include "i2cControl.h"
static const char* TAG = "i2c";

i2cControl::i2cSlave::i2cSlave(gpio_num_t sda_io_pin, gpio_num_t scl_io_pin, uint8_t device_address) : sda{sda_io_pin}, scl{scl_io_pin}, address{device_address}{
    ESP_LOGV(TAG, "SDA pin: %i", (int)sda);
    ESP_LOGV(TAG, "SCL pin: %i", (int)scl);
    ESP_LOGV(TAG, "Device Mode: %i", (int)I2C_MODE_SLAVE);
    ESP_LOGV(TAG, "Device Address: %02x", (int)address);

    config.sda_io_num = sda;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_io_num = scl;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.mode = I2C_MODE_SLAVE;
    config.slave.addr_10bit_en = 0;
    config.slave.slave_addr = address & 0x7F;
    config.slave.maximum_speed = 400000UL;
    config.clk_flags = 0;

    opcode_counter = -1;
    opcode_counter = -1;

    operation = new opcode_t;
    rx_param = new byte;
    tx_buffer_one_byte = new byte;
    tx_buffer = new byte[bufferSize];
}

i2cControl::i2cSlave::~i2cSlave(){
    i2c_driver_delete(i2cPort);

    //de-allocate memory
    // free(operation);
    // free(tx_buffer_one_byte);
    // free(tx_buffer);
    // //free(tx_buffer2);

    delete operation;
    delete rx_param;
    delete tx_buffer_one_byte;
    delete[] tx_buffer;
}

void i2cControl::i2cSlave::init(){
    esp_err_t err;

    err = i2c_param_config(i2cPort, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Parameter Config Failed");
    }
    else ESP_LOGD(TAG, "I2C Paramater Configured");

    err = i2c_driver_install(i2cControl::i2cPort, config.mode, bufferSize, bufferSize, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Driver Install Failed");
    }
    else {
        ESP_LOGI(TAG, "I2C Driver Installed");
        ESP_LOGD(TAG, "Device Mode: %i", (int)config.mode);
        ESP_LOGD(TAG, "Device Address: %02x", (int)address);
    }

    //If software restart by I2C (OpCode 0x21-05), send confirmation to complete communication.
    if(esp_reset_reason() == ESP_RST_SW) {
        ESP_LOGI(TAG, "Software restart complete");
        write_one_byte(i2cControl::validByte);
    }
}

void i2cControl::i2cSlave::reset(){
    esp_err_t err;

    err = i2c_driver_delete(i2cPort);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Driver Delete Failed");
    }
    else ESP_LOGI(TAG, "I2C Driver Deleted");

    init();
}

int i2cControl::i2cSlave::read(byte *rx_data){
    // ESP_LOGV(TAG, "I2C read from buffer");
    return i2c_slave_read_buffer(i2cPort, rx_data, 1, 0);
}

int i2cControl::i2cSlave::write(byte *tx_data){
    // ESP_LOGV(TAG, "I2C wrote to buffer");
    return i2c_slave_write_buffer(i2cPort, tx_data, 1, 0);
}

void i2cControl::i2cSlave::write_one_byte(byte tx_data){
    const int tx_size = _TX_SIZE(1);
    tx_buffer[0] = startByte;
    tx_buffer[1] = tx_data;

    ESP_LOGI(TAG, "Data Response: %02x", (int)tx_data);
    i2c_slave_write_buffer(i2cPort, tx_buffer, tx_size, 0);
}

void i2cControl::i2cSlave::write_one_byte_raw(byte tx_data){
    tx_buffer_one_byte[0] = tx_data;

    if(tx_data != 0xAA){
        ESP_LOGI(TAG, "Byte Sent: %02x", (int)tx_data);
    }
    i2c_slave_write_buffer(i2cPort, tx_buffer_one_byte, 1, 0);
}

void i2cControl::i2cSlave::write_four_bytes(byte4 tx_data){
    const int tx_size = _TX_SIZE(4);

    tx_buffer[0] = startByte;
    tx_buffer[1] = (tx_data & 0xFF000000) >> 24;
    tx_buffer[2] = (tx_data & 0x00FF0000) >> 16;
    tx_buffer[3] = (tx_data & 0x0000FF00) >> 8;
    tx_buffer[4] = (tx_data & 0x000000FF);
    ESP_LOGI(TAG, "Data Response: %02x %02x %02x %02x", tx_buffer[0], tx_buffer[1], tx_buffer[2], tx_buffer[3]);
    
    i2c_slave_write_buffer(i2cPort, tx_buffer, tx_size, 0);
}

void i2cControl::i2cSlave::write_string(std::string tx_data){
    int tx_size = tx_data.length();

    if(tx_size > bufferSize) {
        tx_size = bufferSize;
    }

    for(int i = 0; i < tx_size; i++) {
        tx_buffer[i] = tx_data.at(i);
        ESP_LOGV(TAG, "buffer[%i]", i);
    }

    ESP_LOGD(TAG, "writing i2c");
    ESP_LOGD(TAG, "Data Response: %x", (int)tx_buffer);
    write_one_byte_raw(startByte);
    i2c_slave_write_buffer(i2cPort, tx_buffer, tx_size, 0);
    write_one_byte_raw(endByte);
}

bool i2cControl::i2cSlave::check_for_message(){
    //check for i2c message
    if(bytes_read == 0) {
        // ESP_LOGV(TAG, "checking i2c rx buffer");
        bytes_read += read(operation);
    }

    //if message is received but unprocessed
    if(bytes_read > 0) {
        ESP_LOGI(TAG, "Message Received: %#02x", (int)*operation);
        int paramater_size; //expected size of parameter argument in bytes
        parameter = 0; //reset parameter value

        //operation receipt
        bytes_read -= write(operation);

        //parse parameter length
        paramater_size = *operation >> 5;
        if(paramater_size > 4) paramater_size = 4; //parameter size should be 4 bytes or smaller
        ESP_LOGD(TAG, "OpCode 0x%02x", (int)*operation);
        ESP_LOGD(TAG, "%d-Byte Parameter Expected", paramater_size);
        
        //process parameter if applicable
        if(paramater_size > 0) {
            //receive parameter byte(s)
            while(paramater_size > 0) {
                //check for i2c message
                if(read(rx_param)) {
                    paramater_size--;

                    //combine bytes
                    parameter += *rx_param << (8*paramater_size);
                }
            }
            ESP_LOGI(TAG, "Parameter received: %x", (int)parameter);
        }

        return true;
    }

    return false;
}

void i2cControl::i2cSlave::install_handler(opcode_t opcode, void (*i2c_handler_ptr)(parameter_t)){
    ++opcode_counter;
    opcode_list[opcode_counter] = opcode;
    handler_list[opcode_counter] = i2c_handler_ptr;

    ESP_LOGD(TAG, "%#02x installed with handler", (int)opcode_list[opcode_counter]);
}

void i2cControl::i2cSlave::install_handler(opcode_t opcode){
    ++opcode_ignore_counter;
    opcode_ignore_list[opcode_ignore_counter] = opcode;

    ESP_LOGD(TAG, "%#02x installed without handler", (int)opcode_ignore_list[opcode_ignore_counter]);
}

void i2cControl::i2cSlave::install_handler_unused(void (*i2c_handler_ptr)(parameter_t)){
    handler_invalid = i2c_handler_ptr;
}

void i2cControl::i2cSlave::install_handler_ignore(void (*i2c_handler_ptr)(parameter_t)){
    handler_ignore = i2c_handler_ptr;
}

void i2cControl::i2cSlave::find_handler(opcode_t opcode, parameter_t parameter){
    int opcode_id = -1;

    //check opcode ignore list
    for(int i = 0; i <= opcode_ignore_counter; i++){
        if(opcode_ignore_list[i] == opcode){
            //if there is a match, exit function
            opcode_id = i;
            handler_ignore(parameter);

            ESP_LOGI(TAG, "End Message");

            return;
        }
    }

    //check opcode list
    for(int i = 0; i <= opcode_counter; i++){
        if(opcode_list[i] == opcode){
            opcode_id = i;
            break;
        }
    }

    //call handler if there is a match
    if(opcode_id > -1){
        ESP_LOGD(TAG, "Calling handler for OpCode 0x%02X", (int)opcode_id);
        handler_list[opcode_id](parameter);
    }
    else{
        handler_invalid(parameter);
    }

    ESP_LOGI(TAG, "End Message");
}

void i2cControl::i2cSlave::update(){
    if(check_for_message()){
        find_handler(*operation, parameter);
    }
}