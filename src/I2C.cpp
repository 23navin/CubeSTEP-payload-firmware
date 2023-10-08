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
    log_v("Device Address: %02x", DEVICE_ADDR);

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

    operation = new uint8_t;
    rx_param = new uint8_t;
    tx_buffer_one_byte = new uint8_t;
    tx_buffer = new uint8_t[BUFFER_SIZE];
}

I2cCore::~I2cCore(){
    i2c_driver_delete(I2C_PORT_NUM);

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
        log_d("Device Address: %02x", DEVICE_ADDR);
    }

    //If software restart by I2C (OpCode 0x21-05), send \
    confirmation to complete communication.
    if(esp_reset_reason() == ESP_RST_SW) {
        log_i("Software restart complete");
        write_one_byte(VALID);
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
    // log_v("I2C read from buffer");
    return i2c_slave_read_buffer(0, rx_data, 1, 0);
}

int I2cCore::write(uint8_t *tx_data){
    // log_v("I2C wrote to buffer");
    return i2c_slave_write_buffer(0, tx_data, 1, 0);
}

void I2cCore::write_one_byte(uint8_t tx_data){
    const int tx_size = _TX_SIZE(1);
    tx_buffer[0] = tx_data;

    log_i("Data Response: %02x", tx_data);
    write_one_byte_raw(START_BYTE);
    i2c_slave_write_buffer(0, tx_buffer, tx_size, 0);
}

void I2cCore::write_one_byte_raw(uint8_t tx_data){
    tx_buffer_one_byte[0] = tx_data;

    if(tx_data != 0xAA) log_i("Byte Sent: %02x", tx_data);
    i2c_slave_write_buffer(0, tx_buffer_one_byte, 1, 0);
}

void I2cCore::write_four_bytes(uint32_t tx_data){
    const int tx_size = _TX_SIZE(4);

    tx_buffer[0] = (tx_data & 0xFF000000) >> 24;
    tx_buffer[1] = (tx_data & 0x00FF0000) >> 16;
    tx_buffer[2] = (tx_data & 0x0000FF00) >> 8;
    tx_buffer[3] = (tx_data & 0x000000FF);
    log_i("Data Response: %02x %02x %02x %02x", tx_buffer[0], tx_buffer[1], tx_buffer[2], tx_buffer[3]);
    
    write_one_byte_raw(START_BYTE);
    i2c_slave_write_buffer(0, tx_buffer, tx_size, 0);
}

void I2cCore::write_string(std::string tx_data){
    int tx_size = tx_data.length();

    if(tx_size > BUFFER_SIZE) {
        tx_size = BUFFER_SIZE;
    }

    for(int i = 0; i < tx_size; i++) {
        tx_buffer[i] = tx_data.at(i);
        // log_v("buffer[%i]", i);
    }

    log_d("writing i2c");
    log_d("Data Response: %x", tx_buffer);
    write_one_byte_raw(START_BYTE);
    i2c_slave_write_buffer(0, tx_buffer, tx_size, 0);
    write_one_byte_raw(END_BYTE);

    // const int tx_size = tx_data.length();

    // if(tx_size > TX_SIZE_MAX) {
    //     const int tx2_size = tx_size - TX_SIZE_MAX - 1;

    //     for(int i = 0; i < TX_SIZE_MAX; i++) {
    //         tx_buffer[i] = tx_data.at(i);
    //         log_d("buffer[%i]< %02X", i, tx_buffer[i]);
    //     }

    //     int j = 0;
    //     for(int i = TX_SIZE_MAX; i < tx_size; i++) {
    //         tx_buffer2[j] = tx_data.at(i);
    //         log_d("buffer2[%i]< %02X", j, tx_buffer2[j]);
            
    //         j++;
    //     }
        
    //     log_d("writing i2c");
    //     log_d("Data Response: %X", tx_buffer);
    //     log_d("Data Response: %X", tx_buffer2);
    //     write_one_byte_raw(START_BYTE);
    //     i2c_slave_write_buffer(0, tx_buffer, TX_SIZE_MAX, 0);
    //     i2c_slave_write_buffer(0, tx_buffer2, tx2_size+1, 0);
    //     write_one_byte_raw(END_BYTE);
    // }
    // else {
    //     for(int i = 0; i < tx_size; i++) {
    //         tx_buffer[i] = tx_data.at(i);
    //         log_d("buffer[%i]", i);
    //     }

    //     log_d("writing i2c");
    //     log_d("Data Response: %X", tx_buffer);
    //     write_one_byte_raw(START_BYTE);
    //     i2c_slave_write_buffer(0, tx_buffer, tx_size, 0);
    //     write_one_byte_raw(END_BYTE);
    // }
}

bool I2cCore::check_for_message(){
    //check for i2c message
    if(bytes_read == 0) {
        // log_v("checking i2c rx buffer");
        bytes_read += read(operation);
    }

    //if message is received but unprocessed
    if(bytes_read > 0) {
        log_i("Message Received: %#02x", *operation);
        int paramater_size; //expected size of parameter argument in bytes
        parameter = 0; //reset parameter value

        //operation receipt
        bytes_read -= write(operation);

        //parse parameter length
        paramater_size = *operation >> 5;
        if(paramater_size > 4) paramater_size = 4; //parameter size should be 4 bytes or smaller
        log_d("OpCode 0x%02x", *operation);
        log_d("%d-Byte Parameter Expected", paramater_size);
        
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
            log_i("Parameter received: %x", parameter);
        }

        return true;
    }

    return false;
}

void I2cCore::install_i2c_handler(uint8_t opcode, void (*i2c_handler_ptr)(uint32_t)){
    ++opcode_counter;
    opcode_list[opcode_counter] = opcode;
    handler_list[opcode_counter] = i2c_handler_ptr;

    log_d("%#02x installed with handler", opcode_list[opcode_counter], handler_list[opcode_counter]);
}

void I2cCore::install_i2c_handler(uint8_t opcode){
    ++opcode_ignore_counter;
    opcode_ignore_list[opcode_ignore_counter] = opcode;

    log_d("%#02x installed without handler", opcode_ignore_list[opcode_ignore_counter]);
}

void I2cCore::install_i2c_handler_unused(void (*i2c_handler_ptr)(uint32_t)){
    handler_invalid = i2c_handler_ptr;
}

void I2cCore::install_i2c_handler_ignore(void (*i2c_handler_ptr)(uint32_t)){
    handler_ignore = i2c_handler_ptr;
}

void I2cCore::find_handler(uint8_t opcode, uint32_t parameter){
    int opcode_id = -1;

    //check opcode ignore list
    for(int i = 0; i <= opcode_ignore_counter; i++){
        if(opcode_ignore_list[i] == opcode){
            //if there is a match, exit function
            opcode_id = i;
            handler_ignore(parameter);

            log_i("End Message");

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
        log_d("Calling handler for OpCode 0x%02X", opcode_id);
        handler_list[opcode_id](parameter);
    }
    else{
        handler_invalid(parameter);
    }

    log_i("End Message");
}

void I2cCore::update(){
    if(check_for_message()){
        find_handler(*operation, parameter);
    }
}