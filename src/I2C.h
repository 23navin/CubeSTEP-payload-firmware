/**
 * @file I2C.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief I2C interface
 * 
 */

#ifndef _i2c_H_included
#define _i2c_H_included

#include "ESP32Time.h"
#include "driver/i2c.h"

#define I2C_SDA_PIN GPIO_NUM_19
#define I2C_SCL_PIN GPIO_NUM_23
#define I2C_PORT_NUM I2C_NUM_0
#define SLAVE_ADDR 0x23

#define BUFFER_SIZE 256 //Size of I2C rx and tx buffers
#define DATA_SIZE 4 //Size of I2C data response
#define TX_SIZE (DATA_SIZE + 1) //Size of I2C data transmit

#define START_BYTE 0xFF

#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave rx buffer size */

/**
 * @brief 
 * 
 */
class I2cCore{
public:
    uint32_t response;

    I2cCore();
    ~I2cCore();
    void init();
    inline int getAddress(){
        return address;
    }
    int read(uint8_t *rx_data);
    int write(uint8_t *tx_data);
    void write_four_bytes(uint32_t tx_data);
    bool check_for_message();
    inline uint8_t get_opcode(){
        return *operation;
    }
    inline uint64_t get_parameter(){
        return parameter;
    }
    void install_procedure_handler(uint8_t opcode, void (*)());
    void install_procedure_handler(uint8_t opcode, void (*)(uint8_t));


private:
    //I2C Config
    i2c_config_t config;
    gpio_num_t sda = I2C_SDA_PIN;
    gpio_num_t scl = I2C_SCL_PIN;
    uint16_t address = SLAVE_ADDR;

    //Rx Data
    uint8_t *operation = (uint8_t *)malloc(sizeof(uint8_t)); //opcode
    uint64_t parameter; //parameter to operation
    int bytes_read = 0; //bytes read off i2c

};

#endif // _i2c_H_included