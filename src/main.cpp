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
 * @brief Saves system snapshot to Telemetry object and saves it to log
 * 
 * @param file_p 
 * @param sens_p 
 * @param pwm_p 
 */
void capture_telemetry_to_log(FileCore *file_p, SensorCore *sens_p, HeaterCore *pwm_p){
    Telemetry capture;
    sens_p->snapshot(&capture);
    pwm_p->snapshot(&capture);
    addLine(file_p, &capture);
}

ESP32Time rtc;
FileCore storage;
SensorCore sensor;
HeaterCore heater;
I2cCore slave;

//I2C call functions

/**
 * @brief Function called when an undefined opcode is
 * received.
 * 
 * @param parameter unused
 */
void i2c_handler_unused(uint32_t parameter){
    log_w("Invalid Opcode");
}

/**
 * @brief OpCode: 0x21
 * @note Performs a software reset. When the function is 
 * called, execution of the program stops, both CPUs are 
 * reset, the application is loaded by the bootloader and 
 * starts execution again. 
 * 
 * @param parameter unused
 */
void i2c_handler_restart_device(uint32_t parameter){
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
void i2c_handler_wake_device(uint32_t parameter);

/**
 * @brief OpCode 0x02
 * @note Puts device in light sleep mode. In Light-sleep 
 * mode, the digital peripherals, most of the RAM, and 
 * CPUs are clock-gated and their supply voltage is 
 * reduced. Upon exit from Light-sleep, the digital 
 * peripherals, RAM, and CPUs resume operation and their 
 * internal states are preserved.
 * 
 * @param parameter unused
 */
void i2c_handler_sleep_device(uint32_t parameter){
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
    i2c_handler_wake_device(INVALID);
}

/**
 * @brief OpCode 0x03
 * @note Device will wake up from sleep mode when the
 * SCL pin is brought to low. 
 * 
 * @param parameter unused
 */
void i2c_handler_wake_device(uint32_t parameter){
    log_i("waking up...");
    slave.write_one_byte(DEVICE_ADDR);

    heater.resumePWM(); //for debug remove/change as needed
    log_i("End Message");
}

/**
 * @brief OpCode 0x04
 * @note 
 * 
 * @param parameter unused
 */
void i2c_handler_run_system_check(uint32_t parameter){

    log_i("End Message");
}

/**
 * @brief Opcode 0x25
 * @note Device will run a brief system check on the
 * specified system and return a metric related to
 * its performance.
 * 
 * @param 0x54 PWM Signal Generator
 * @param 0x49 I2C Slave
 * @param 0x7C SPI Flash
 * @param 0x25 ADC Temperature Sensors
 */
void i2c_handler_get_system_check_result(uint32_t parameter){

    log_i("End Message");
}

/**
 * @brief OpCode 0x0F
 * @note Prepares experiment log file to be read by the
 * I2C system. This function must be called before get_log
 * 
 * @param parameter unused
 */
void i2c_handler_prepare_log(uint32_t parameter){
    // log_buffer = storage.loadFile("/log.csv");
    // slave.write_one_byte(log_buffer.size());

    storage.select_file("/log.csv");
    slave.write_one_byte(0x0F);
}

/**
 * @brief OpCode 0x11
 * @note Returns an individual line from the experiment
 * log file. This function must be called after prepare_log.
 * The first call will return the first line, the second
 * call will return the second line, etc... If there is no
 * line to send, it will return INVALID and close the file.
 * 
 * @param parameter unused
 */
void i2c_handler_get_log(uint32_t parameter){
    string buffer;
    int line;

    line = storage.read_file(&buffer);
    if(line != -1) {
        slave.write_string(buffer);
    }
    else {
        storage.deselect_file();
        
        slave.write_one_byte(INVALID);
    }
    
}

/**
 * @brief OpCode 0x32
 * @note Returns current time of the device from real time 
 * clock object.
 * 
 * @param 0xEE Returns Epoch
 * @param 0x4D Returns MilliSecond
 */
void i2c_handler_get_time(uint32_t parameter){
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
 * @note Sets the current time of the device
 * 
 * @param parameter Epoch
 */
void i2c_handler_set_time(uint32_t parameter){
    log_i("System time sync");
    log_d("System time set to %x", parameter);
    rtc.setTime(parameter);

    slave.write_four_bytes(parameter);
    log_i("End Message");
}

/**
 * @brief Opcode 0x34
 * @note Returns the temperature of a provided
 * Sensor
 * 
 * @param 0x00 Sensor 0
 * @param 0x01 Sensor 1
 * @param ....
 * @param 0x0F Sensor 15 
 */
void i2c_handler_get_temperature(uint32_t parameter){
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

/**
 * @brief OpCode 0x15
 * @note Returns the current PWM Duty value
 * 
 * @param parameter 
 */
void i2c_handler_get_pwm_duty(uint32_t parameter){
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

    // i2c setup
    slave.init();

    // PWM setup
    heater.initPWM();
    heater.startPWM();
    heater.setDutyCycle(40, 1);

    // SPI Flash testing
    Telemetry active, capture;
    storage.listDir("/", 0);
    deleteLog(&storage);
    createLog(&storage, &active);
    storage.listDir("/", 0);

    capture_telemetry_to_log(&storage, &sensor, &heater);

    heater.pausePWM();
    capture_telemetry_to_log(&storage, &sensor, &heater);

    heater.resumePWM();
    heater.setDutyCycle(90, 4);
    capture_telemetry_to_log(&storage, &sensor, &heater);

    // log_i("SPI Flash tested at %i ms",storage.testFileIO("/test.txt"));
    // log_i("ADC test result: %f", sensor.test());

    //new file
    int line_count;
    storage.select_file("/log.csv");

    do{
        string read_buffer;

        line_count = storage.read_file(&read_buffer);
        Serial.println(read_buffer.c_str());
    } while(line_count != -1);

    storage.deselect_file();

    //define i2c handler call functions
    slave.install_i2c_handler_unused(i2c_handler_unused);
    slave.install_i2c_handler(0x21, i2c_handler_restart_device);
    slave.install_i2c_handler(0x02, i2c_handler_sleep_device);
    slave.install_i2c_handler(0x0F, i2c_handler_prepare_log);
    slave.install_i2c_handler(0x11, i2c_handler_get_log);
    slave.install_i2c_handler(0x32, i2c_handler_get_time);
    slave.install_i2c_handler(0x93, i2c_handler_set_time);
    slave.install_i2c_handler(0x34, i2c_handler_get_temperature);
    slave.install_i2c_handler(0x15, i2c_handler_get_pwm_duty);
    
    heater.setDutyCycle(50, 0.5);
    log_i("Setup completed.");
}

void loop(){
    slave.update();
}