/**
 * @file main.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief ESP32 Payload Processor firmware
 * //will add documentation later 
*/

using namespace std;
#include "ESP32Time.h"
#include "esp_log.h"
#include "esp_err.h"

#include "File.h"
#include "Telemetry.h"
#include "Sensor.h"
#include "Heater.h"
#include "I2C.h"

#define one_second (1000) //one thousand milli-seconds

//Logging
#define TAGI "payload_INFO"

//UART
#define UART_BAUD_RATE 115200 //default baud rate

/**
 * @brief Sets a time object to the system time when this build was compiled
 * @note Useful as a baseline time variable
 * 
 * @param object ESP32Time pointer
 */
void setTimeToCompile(ESP32Time *object)
{
    char s_month[5];
    int year;
    struct tm t;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(__DATE__, "%s %d %d", s_month, &t.tm_mday, &year);
    sscanf(__TIME__, "%2d %*c %2d %*c %2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
    t.tm_mon = (strstr(month_names, s_month) - month_names) / 3;    
    t.tm_year = year - 1900;   

    object->setTime(mktime(&t));
}

/**
 * @brief Creates a Log object
 * @note for testing
 * 
 * @param file_p 
 * @param tele 
 */
void createLog(FileCore *file_p, Telemetry *tele){
    char header[1000];
    tele->headerCSV(header);
    file_p->writeFile("/log.csv", header);
}

void deleteLog(FileCore *file_p) {
    file_p->deleteFile("/log.csv");
}

/**
 * @brief Adds line to file
 * @note for testing
 * 
 * @param file_p 
 * @param tele 
 */
void addLine(FileCore *file_p, Telemetry *tele){
    char line[line_size];
    tele->ToCSV(line);
    file_p->appendFile("/log.csv", line);
}

/**
 * @brief Initializes file system
 * @note for testing
 * 
 * @param file_p 
 */
void initSPI(FileCore *file_p){
    file_p->mount();
    //createLog();
}

void PWMtest(HeaterCore *heater_p) {
    int cycle = 0;
    while(1){
        // Telemetry test;
        // sensor.snapshot(&test);
        delay(5000);

        heater_p->setDutyCycle(cycle);

        cycle+=10;
        if(cycle > 100) cycle = 0;
    }
}

void fileTester(FileCore *file_p){
    vector<String> recovered;
    recovered = file_p->loadFile("/log.csv");
    
    Serial.println(recovered.size());
    for(int i=0; i < recovered.size(); i++) {
        Serial.printf("Line #%i\n", i);
        Serial.println(recovered[i]);
    }
}

ESP32Time rtc;
FileCore storage;
SensorCore sensor;
HeaterCore heater;
I2cCore slave;

//I2C call functions

void i2c_handler_unused(){
    log_w("Invalid Opcode");
}

/**
 * @brief OpCode: 0x21
 * @note Performs a software reset. When the function is 
 * called, execution of the program stops, both CPUs are 
 * reset, the application is loaded by the bootloader and 
 * starts execution again. 
 */
void i2c_handler_restart_device(uint8_t parameter){
    if(parameter == 0x49) {
        log_i("Restarting I2C");
        slave.reset();

        slave.write_one_byte(parameter);
    }
    else if(parameter == 0x05) {
        log_i("Restarting Device");
        esp_restart();
    }
    else {
        log_w("Invalid Parameter");
        slave.write_one_byte(0xFF);
    }
    log_i("End Message");
}

//declare 0x03 handler so that it can be used by 0x02
void i2c_handler_wake_device();

/**
 * @brief OpCode 0x02
 * @note Puts device in light sleep mode. In Light-sleep 
 * mode, the digital peripherals, most of the RAM, and 
 * CPUs are clock-gated and their supply voltage is 
 * reduced. Upon exit from Light-sleep, the digital 
 * peripherals, RAM, and CPUs resume operation and their 
 * internal states are preserved.
 */
void i2c_handler_sleep_device(){
    //set up sleep mode
    gpio_wakeup_enable(I2C_SCL_PIN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    //turn off any GPIO outputs
    heater.pausePWM();

    //put device to sleep
    log_i("going to sleep...");
    log_i("End Message");
    delay(10);
    esp_light_sleep_start();

    //device will wake when the i2c bus is used. OpCode\
    0x03 (Wake Device)'s should be used to wake the device\
    so it's i2c handler is run immediately upon waking.
    i2c_handler_wake_device();
}

/**
 * @brief OpCode 0x03
 * @note Device will wake up from sleep mode when the
 * SCL pin is brought to low. 
 * 
 */
void i2c_handler_wake_device(){
    log_i("waking up...");
    slave.write_one_byte(DEVICE_ADDR);

    heater.resumePWM(); //for debug remove/change as needed
    log_i("End Message");
}

/**
 * @brief OpCode 0x04
 */
void i2c_handler_run_system_check(){

    log_i("End Message");
}

void i2c_handler_get_system_check_result(uint8_t parameter){

    log_i("End Message");
}

/**
 * @brief OpCode 0x32
 * @note Returns current time of the device from real time 
 * clock object.
 * 
 * @param 0xEE Returns Epoch
 * @param 0x4D Returns MilliSecond
 */
void i2c_handler_get_time(uint8_t parameter){
    if(parameter == 0xEE) { //Epoch
        slave.write_four_bytes(rtc.getEpoch());
        }

    else if (parameter == 0x4D) { //ms
        slave.write_four_bytes(rtc.getMillis());
    }
    else { //Invalid Parameter
        log_w("Invalid Parameter");
        slave.write_one_byte(0xFF);
    }
    log_i("End Message");
}

/**
 * @brief OpCode 0x93
 * 
 * @param parameter Epoch
 */
void i2c_handler_set_time(uint64_t parameter){
    log_i("System time sync");
    log_d("System time set to %X", parameter);
    rtc.setTime(parameter);

    slave.write_four_bytes(parameter);
    log_i("End Message");
}

/**
 * @brief 
 * 
 * @param parameter 
 */
void i2c_handler_get_temperature(uint8_t parameter){
    if(parameter >= number_of_temp_sensors) {
        slave.write_one_byte(INVALID);
        log_d("%i vs %i", parameter, number_of_temp_sensors);

    }
    else {
        float temperature;
        temperature = sensor.sample(parameter);
        union {
            float float_data;
            uint32_t uint_data;
        } float_to_uint = { .float_data = temperature };
        
        slave.write_four_bytes(float_to_uint.uint_data);
    }
    log_i("End Message");
}

void i2c_handler_get_pwm_duty(){
    slave.write_one_byte(heater.getDutyCycle());
    log_i("End Message");
}

void setup(){

    //set internal rtc clock
    setTimeToCompile(&rtc);
    sensor.init(&rtc);

    // Serial setup
    Serial.begin(UART_BAUD_RATE);
    delay(1000);
    log_i("Communication started over UART @ %i", UART_BAUD_RATE);
    log_d("System time is %i/%i\n", rtc.getEpoch(), rtc.getMillis());

    // SPI Flash testing
    // Telemetry active, capture;
    // Serial.println("file tests below");
    // storage.listDir("/", 0);
    // deleteLog(&storage);
    // createLog(&storage, &active);
    // storage.listDir("/", 0);

    // sensor.snapshot(&capture);
    // addLine(&storage, &capture);

    // active.random(&rtc);
    // addLine(&storage, &active);

    // log_i("SPI Flash tested at %i ms",storage.testFileIO("/test.txt"));
    // log_i("ADC test result: %f", sensor.test());

    fileTester(&storage);
    heater.initPWM();
    heater.startPWM();
    heater.setDutyCycle(40, 1);

    //i2c setup
    slave.init();

    log_i("Setup completed.");
}

void loop(){
    if(slave.check_for_message()) {
        uint8_t opcode = slave.get_opcode();
        uint32_t parameter = slave.get_parameter();

        //00 - 
        if(opcode == 0x00) /*i2c_handler*/;

        //01 - Restart Device
        if(opcode == 0x21) i2c_handler_restart_device(parameter);

        //02 - Sleep Device
        if(opcode == 0x02) i2c_handler_sleep_device();

        //03 - Wake Device (handler is included in <2 - Sleep Device> and called \
              automatically upon wakeup from sleep)

        //04 - 
        if(opcode == 0x04) /*i2c_handler*/;

        //05 - 
        if(opcode == 0x05) /*i2c_handler*/;

        //06 - 
        if(opcode == 0x06) /*i2c_handler*/;

        //07 - 
        if(opcode == 0x07) /*i2c_handler*/;

        //08 - 
        if(opcode == 0x08) /*i2c_handler*/;

        //09 - 
        if(opcode == 0x09) /*i2c_handler*/;

        //0A - 
        if(opcode == 0x0A) /*i2c_handler*/;

        //0B - 
        if(opcode == 0x0B) /*i2c_handler*/;

        //0C - 
        if(opcode == 0x0C) /*i2c_handler*/;

        //0D - 
        if(opcode == 0x0D) /*i2c_handler*/;

        //0E - 
        if(opcode == 0x0E) /*i2c_handler*/;

        //0F - Get Experiment Log Length
        if(opcode == 0x0F) /*i2c_handler*/;

        //10 - 
        if(opcode == 0x10) /*i2c_handler*/;

        //11 - 
        if(opcode == 0x11) /*i2c_handler*/;

        //12 - 
        if(opcode == 0x12) /*i2c_handler*/;

        //12 - Get Time
        if(opcode == 0x32) i2c_handler_get_time(parameter);
        
        //13 - Set Time
        if(opcode == 0x93) i2c_handler_set_time(parameter);

        //14 - Get Temperature
        if(opcode == 0x34) i2c_handler_get_temperature(parameter);

        //15 - Get PWM Duty
        if(opcode == 0x15) i2c_handler_get_pwm_duty();

        //16 - 
        if(opcode == 0x16) /*i2c_handler*/;

        //17 - 
        if(opcode == 0x17) /*i2c_handler*/;

        //18 - 
        if(opcode == 0x18) /*i2c_handler*/;

        //19 - 
        if(opcode == 0x19) /*i2c_handler*/;

        //1A - 
        if(opcode == 0x1A) /*i2c_handler*/;

	    //1B - 
        if(opcode == 0x1B) /*i2c_handler*/;

        //1C - 
        if(opcode == 0x1C) /*i2c_handler*/;

        //1D - 
        if(opcode == 0x1D) /*i2c_handler*/;

        //1E - 
        if(opcode == 0x1E) /*i2c_handler*/;

        //1F - 
        if(opcode == 0x1F) /*i2c_handler*/;
    }
}

//i2c demo

// uint8_t *operation = (uint8_t *)malloc(sizeof(uint8_t)); //opcode
// uint64_t parameter; //parameter to operation
// int bytes_read = 0; //bytes read off i2c

// while(1){
//     //check for i2c message
//     if(bytes_read == 0) {
//         bytes_read += i2c_slave_read_buffer(0, operation, 1, 0);
//     }

//     //if message is received but unprocessed
//     if(bytes_read > 0) {
//         int paramater_size; //expected size of parameter argument in bytes
//         parameter = 0; //reset parameter value


//         //operation receipt
//         bytes_read -= i2c_slave_write_buffer(0, operation, 1, 0);

//         //parse parameter length
//         paramater_size = *operation >> 5;
//         if(paramater_size > 4) paramater_size = 4; //parameter size should be 4 bytes or smaller
//         Serial.printf("Message received 0x%02X -> %d-Byte Parameter", *operation, paramater_size);
        
//         //process parameter if applicable
//         if(paramater_size > 0) {
//             uint8_t *rx_data = (uint8_t *)malloc(sizeof(uint8_t)); //byte received over i2c

//             //receive parameter byte(s)
//             while(paramater_size > 0) {
//                 //check for i2c message
//                 if(i2c_slave_read_buffer(0, rx_data, 1, 0)) {
//                     paramater_size--;

//                     //combine bytes
//                     parameter += *rx_data << (8*paramater_size);
//                 }
//             }
//             Serial.printf("Parameter received: 0x%X", parameter);
//         }

//         //return data if applicable
//         if(*operation == 0x32) { //Get Time
//             if(parameter == 0xEE) { //Epoch
//                 uint32_t buffer;
//                 int data_size = 4; //number of bytes to be transmitted
//                 int tx_size = data_size+1; //number of bytes to be transmitted including start byte
//                 uint8_t *tx_data = (uint8_t *)malloc((tx_size)*sizeof(uint8_t)); //data to transmit
                
//                 //get data
//                 buffer = rtc.getEpoch();

//                 //split data into individual bytes
//                 tx_data[0] = 0xFF; //start byte
//                 tx_data[1] = (buffer & 0xFF000000) >> 24;
//                 tx_data[2] = (buffer & 0x00FF0000) >> 16;
//                 tx_data[3] = (buffer & 0x0000FF00) >> 8;
//                 tx_data[4] = (buffer & 0x000000FF);
//                 Serial.printf("Data Response: %X -> %02X %02X %02X %02X", buffer, tx_data[1], tx_data[2], tx_data[3], tx_data[4]);

//                 //send data
//                 i2c_slave_write_buffer(0, tx_data, tx_size, 0);
//             }
//             else if (parameter == 0x4D) { //ms
//             }
//             else { //Invalid Parameter

//             }
//         }
//         Serial.println("End Message");
//     }
// }
