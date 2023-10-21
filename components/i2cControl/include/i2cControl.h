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

#include "driver/i2c.h"
#include <string>

#define _TX_SIZE(DATA_SIZE) (DATA_SIZE + 1) //Size of I2C data transmit


namespace i2cControl{
    //data types
    typedef unsigned char byte;
    typedef unsigned long byte4;
    typedef unsigned char opcode_t;
    typedef unsigned long parameter_t;

    //i2c hardware
    constexpr i2c_port_t i2cPort = I2C_NUM_0;

    //i2c buffers
    constexpr int bufferSize = 256; //Size of I2C rx and tx buffers
    constexpr int opcodeListSize = 32;

    //special bytes
    constexpr byte startByte = 0xAA;
    constexpr byte endByte = 0x04;
    constexpr byte validByte = 0x88;
    constexpr byte invalidByte = 0xFF;
    constexpr byte unknownByte = 0x44;

    //call function type
    typedef void(*function_ptr)(parameter_t);
    

    /**
     * @brief Interface to read and write to I2C Bus
     * @note - Scan Rx buffer for incoming messages
     * @note - Parse and process messages
     * @note - Limited framework to create a response
     */
    class i2cSlave{
    public:
        i2cSlave(gpio_num_t sda_io_pin, gpio_num_t scl_io_pin, uint8_t device_address);
        ~i2cSlave();
        inline gpio_num_t get_sda_io_pin(){
            return sda;
        }
        inline gpio_num_t get_scl_io_pin(){
            return scl;
        }
        inline uint8_t get_device_address(){
            return address;
        }
        void init();
        void reset();
        inline int getAddress(){
            return address;
        }
        void write_one_byte(byte tx_data);
        void write_one_byte_raw(byte tx_data);
        void write_four_bytes(byte4 tx_data);
        void write_string(std::string tx_data);
        bool check_for_message();
        inline opcode_t get_opcode(){
            return *operation;
        }
        inline parameter_t get_parameter(){
            return parameter;
        }
        void install_handler(opcode_t opcode);
        void install_handler(opcode_t opcode, void (*i2c_handler_ptr)(parameter_t));
        void install_handler_unused(void (*i2c_handler_ptr)(parameter_t));
        void install_handler_ignore(void (*i2c_handler_ptr)(parameter_t));
        void find_handler(opcode_t opcode, parameter_t parameter);
        void update();

    private:
        //I2C Config
        i2c_config_t config;
        gpio_num_t sda;
        gpio_num_t scl;
        uint8_t address;

        //Rx Data
        opcode_t *operation; //opcode
        byte *rx_param; //parameter read buffer
        parameter_t parameter; //parameter to operation
        
        int bytes_read = 0; //bytes read off i2c

        //Tx Data
        byte *tx_buffer_one_byte; //opcode
        byte *tx_buffer;

        int messages_sent = 0;

        int read(byte *rx_data);
        int write(byte *tx_data);

        //call function handler
        opcode_t opcode_list[opcodeListSize];
        opcode_t opcode_ignore_list[opcodeListSize];
        function_ptr handler_list[opcodeListSize];
        function_ptr handler_invalid;
        function_ptr handler_ignore;
        int opcode_counter;
        int opcode_ignore_counter;
    };
}

#endif // _i2c_H_included