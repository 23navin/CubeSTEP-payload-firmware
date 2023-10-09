/**
 * @file I2C.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief I2C interface
 * 
 */

#ifndef _i2c_H_included
#define _i2c_H_included

#include "esp_log.h"
#include "esp_err.h"

#include "ESP32Time.h"
#include "driver/i2c.h"
#include <string>

#define I2C_SDA_PIN         GPIO_NUM_19
#define I2C_SCL_PIN         GPIO_NUM_23
#define I2C_PORT_NUM        I2C_NUM_0
#define DEVICE_ADDR         0x23

#define BUFFER_SIZE         256 //Size of I2C rx and tx buffers
#define _TX_SIZE(DATA_SIZE) (DATA_SIZE + 1) //Size of I2C data transmit

#define START_BYTE          0xAA
#define END_BYTE            0x04
#define VALID               0x88
#define INVALID             0xFF
#define UNKNOWN             0x44

typedef uint8_t opcode_t;
typedef uint32_t parameter_t;

//call function handlers
#define OPCODE_LIST_SIZE    32
typedef void(*function_ptr)(uint32_t);


/**
 * @brief Interface to read and write to I2C Bus
 * @note - Scan Rx buffer for incoming messages
 * @note - Parse and process messages
 * @note - Limited framework to create a response
 */
class I2cCore{
public:
    uint32_t response;
    I2cCore();
    ~I2cCore();
    void init();
    void reset();
    inline int getAddress(){
        return address;
    }
    void write_one_byte(uint8_t tx_data);
    void write_one_byte_raw(uint8_t tx_data);
    void write_four_bytes(uint32_t tx_data);
    void write_string(std::string tx_data);
    bool check_for_message();
    inline uint8_t get_opcode(){
        return *operation;
    }
    inline uint64_t get_parameter(){
        return parameter;
    }
    void install_handler(uint8_t opcode);
    void install_handler(uint8_t opcode, void (*i2c_handler_ptr)(uint32_t));
    void install_handler_unused(void (*i2c_handler_ptr)(uint32_t));
    void install_handler_ignore(void (*i2c_handler_ptr)(uint32_t));
    void find_handler(uint8_t opcode, uint32_t parameter);
    void update();

private:
    //I2C Config
    i2c_config_t config;
    gpio_num_t sda = I2C_SDA_PIN;
    gpio_num_t scl = I2C_SCL_PIN;
    uint32_t address = DEVICE_ADDR;

    //Rx Data
    uint8_t *operation; //opcode
    uint8_t *rx_param; //parameter read buffer
    uint32_t parameter; //parameter to operation
    
    int bytes_read = 0; //bytes read off i2c

    //Tx Data
    uint8_t *tx_buffer_one_byte; //opcode
    uint8_t *tx_buffer;
    // uint8_t *tx_buffer2;

    int messages_sent = 0;

    int read(uint8_t *rx_data);
    int write(uint8_t *tx_data);

    //call function handler
    uint8_t opcode_list[OPCODE_LIST_SIZE];
    uint8_t opcode_ignore_list[OPCODE_LIST_SIZE];
    function_ptr handler_list[OPCODE_LIST_SIZE];
    function_ptr handler_invalid;
    function_ptr handler_ignore;
    int opcode_counter;
    int opcode_ignore_counter;
};

#endif // _i2c_H_included