/**
 * @file main.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief CubeSTEP - Payload Processor - Firmware for ESP32
*/

using namespace std;
#include <string.h>

#include "esp_log.h"
#include "esp_err.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_sleep.h" //power management
#include "esp_sntp.h" //system time

#include "i2cControl.h"
#include "adcControl.h"
#include "pwmControl.h"
#include "fileControl.h"
#include "experimentControl.h"
#include "telemetryControl.h"

//Logging
#define TAG "system"
#define TAG_i2c "i2c_handle"
#define TAG_task "task"

//UART
#define UART_BAUD_RATE 115200 //default baud rate

//SPI
#define LOG_FILE_NAME "/spiffs/exp_log.csv"

/* Support Functions */

/**
 * @brief Sets a time object to the system time when this build was compiled
 * @note Useful as a baseline time variable
 * 
 */
void set_system_time_to_compile()
{
    char s_month[5];
    int year;
    struct tm t;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(__DATE__, "%s %d %d", s_month, &t.tm_mday, &year);
    sscanf(__TIME__, "%2d %*c %2d %*c %2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
    t.tm_mon = (strstr(month_names, s_month) - month_names) / 3;    
    t.tm_year = year - 1900;   

    uint32_t epoch = mktime(&t);
    struct timeval tv;
    if (epoch > 2082758399){
        tv.tv_sec = epoch - 2082758399;  // epoch time (seconds)
    } else {
        tv.tv_sec = epoch;  // epoch time (seconds)
    }
    tv.tv_usec = 0;    // microseconds
    settimeofday(&tv, NULL);
}

/* Objects */

FileCore storage;
SensorCore sensor;
HeaterCore pwm;
I2cCore slave;

Experiment payload;
Telemetry active;

/* Task Handles */

TaskHandle_t exp_run_task = NULL;
TaskHandle_t exp_log_task = NULL;
TaskHandle_t exp_plog_task = NULL;

/* Tasks */

/**
 * @brief Task that scans i2c bus for message. If message received,
 * process opcode and parameter handshake interaction. Calls
 * i2c_handler associated to opcode. Task does not self-delete.
 * 
 * @param pvParameters none
 */
void i2c_scan(void *pvParameters){
    (void)pvParameters;

    while(1){
        slave.update();
    }
}

/**
 * @brief Task that runs experiment procedure as defined by the
 * Experiment struct. Logs telemetry data to SPI Flash
 * storage throughout procedure. Task deletes upon experiment
 * completion.
 * 
 * @param pvParameters none
 */
void exp_run(void *pvParameters){
    (void)pvParameters;

    while(1){
        //check if experiment is already running
        if(payload.status){
            //exit
        }

        //indicate that experiment is active
        ESP_LOGI(TAG_task, "Experiment Starting");
        payload.status = EXP_STARTUP;

        //reset current stage counter
        payload.current_stage = 0;

        //log baseline
        ESP_LOGI(TAG_task, "Logging Baseline (%i ms)", (int)payload.startup_length);
        vTaskDelay(payload.startup_length / portTICK_PERIOD_MS);

        //reset pwm out
        pwm.setPWM(payload.pwm_period, 0);
        pwm.startPWM();

        payload.status = EXP_ACTIVE;

        //stage loop
        while(payload.current_stage < payload.stage_count){
            //check for experiment exit
            if(payload.stop_flag){
                payload.stop_flag = false;
                pwm.pausePWM();
                break;
            }

            //set pwm to stage pwm param
            ESP_LOGI(TAG_task, "Advancing to Stage %i (%i ms)", (int)payload.current_stage, (int)payload.length[payload.current_stage]);
            pwm.setDutyCycle(payload.pwm_duty[payload.current_stage]);

            //wait stage length
            vTaskDelay(payload.length[payload.current_stage] / portTICK_PERIOD_MS);
            
            payload.current_stage++;
        }

        //turn off pwm
        pwm.pausePWM();

        payload.status = EXP_COOLDOWN;

        //post-experiment log
        ESP_LOGI(TAG_task, "Logging cooldown (%i ms)", (int)payload.cooldown_length);
        vTaskDelay(payload.cooldown_length / portTICK_PERIOD_MS);

        //turn logger off
        while(payload.logger_status == true){
        }
        vTaskDelete(exp_log_task);

        //exit task
        ESP_LOGI(TAG_task, "Experiment Completed");
        payload.status = EXP_INACTIVE;
        vTaskDelete(NULL);
    }
}

/**
 * @brief Task that discretely records system telemetry to SPI
 * Flash storage on a set inerval. Task does not self-delete.
 * 
 * @param pvParameters 
 */
void exp_log(void *pvParameters){
    uint32_t *interval = (uint32_t *) pvParameters;
    const TickType_t xDelay = *interval / portTICK_PERIOD_MS;
    ESP_LOGI(TAG_task, "Logger started: interval %ims", (int)*interval);
    
    //objects to hold log data
    Telemetry capture;
    char line[line_size];

    while(1){
        //set logger status as active
        payload.logger_status = true;

        //capture time data
        struct timeval tv;
        gettimeofday(&tv, NULL);
        capture.setTime(tv.tv_sec, tv.tv_usec);

        //capture temperature data
        for(int sensor_number = 0; sensor_number < number_of_temp_sensors; sensor_number++){
            //Gather data
            float buffer = sensor.sample(sensor_number);

            //Put data into Telemetry object
            capture.setTemp(sensor_number, buffer);
        }

        //capture heater data
        if(pwm.getStatus()){
            //if pwm is on
            capture.setPWM(pwm.getDutyCycle(), pwm.getCyclePeriod());
        }
        else{
            //if pwm is off
            capture.setPWM(0, pwm.getCyclePeriod());
        }

        //log telemetry
        capture.ToCSV(line);
        storage.addLine(LOG_FILE_NAME, line);

        //set logger status as inactive
        payload.logger_status = false;

        //wait interval
        vTaskDelay(xDelay);
    }
}

/* I2C Call Functions */

/**
 * @brief Function called when an undefined opcode is
 * received.
 * 
 * @return nothing
 */
void i2c_unused(uint32_t parameter){
    ESP_LOGW(TAG_i2c, "Undefined Opcode");
}

/**
 * @brief Function called when an opcode is ignored
 * 
 * @return nothing
 */
void i2c_ignore(uint32_t parameter){
    ESP_LOGI(TAG_i2c, "OpCode Ignored");
}

/**
 * @brief OpCode: 0x21
 * @note Performs a software reset. When the function is 
 * called, execution of the program stops, both CPUs are 
 * reset, the application is loaded by the bootloader and 
 * starts execution again. 
 * 
 * @param 0x49 Restart I2C
 * @param 0x05 Restart Device
 * 
 * @return VALID upon successful reset;
 * @return UNKNOWN if undefined parameter
 */
void i2c_restart_device(uint32_t parameter){
    if(parameter == 0x49) { //Restart I2C
        ESP_LOGI(TAG_i2c, "Restarting I2C");
        slave.reset();

        slave.write_one_byte(VALID);
    }
    else if(parameter == 0x05) { //Restart Device
        ESP_LOGI(TAG_i2c, "Restarting Device");
        esp_restart();
    }
    else {
        ESP_LOGW(TAG_i2c, "Invalid Parameter");
        slave.write_one_byte(UNKNOWN);
    }
}


/**
 * @brief OpCode 0x03
 * @note Device will wake up from sleep mode when the
 * SCL pin is brought to low. 
 * 
 * @param _unused
 * 
 * @return VALID upon wake
 */
void i2c_wake_device(uint32_t parameter);

/**
 * @brief OpCode 0x02
 * @note Puts device in light sleep mode. In Light-sleep 
 * mode, the digital peripherals, most of the RAM, and 
 * CPUs are clock-gated and their supply voltage is 
 * reduced. Upon exit from Light-sleep, the digital 
 * peripherals, RAM, and CPUs resume operation and their 
 * internal states are preserved.
 * 
 * @param _unused
 * 
 * @return VALID before sleep
 */
void i2c_sleep_device(uint32_t parameter){
    //set up sleep mode
    gpio_wakeup_enable(I2C_SCL_PIN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    //turn off any GPIO outputs
    pwm.pausePWM();

    slave.write_one_byte(VALID);

    //put device to sleep
    ESP_LOGI(TAG_i2c, "going to sleep...");
    vTaskDelay(10 / portTICK_PERIOD_MS);
    esp_light_sleep_start();

    /*device will wake when the i2c bus is used. OpCode
    0x03 (Wake Device)'s should be used to wake the device
    so it's i2c handler is run immediately upon waking.*/
    i2c_wake_device(VALID);
}

//declared earlier so that it can be used by 0x02
void i2c_wake_device(uint32_t parameter){
    ESP_LOGI(TAG_i2c, "waking up...");
    slave.write_one_byte(DEVICE_ADDR);
}

/**
 * @brief OpCode 0x04 (deprecated)
 * @note 
 * 
 * @param _unused
 * 
 * @return
 */
void i2c_run_system_check(uint32_t parameter){

}

/**
 * @brief Opcode 0x25 (deprecated)
 * @note Device will run a brief system check on the
 * specified system and return a metric related to its
 * performance.
 * 
 * @param 0x54 PWM Signal Generator
 * @param 0x49 I2C Slave
 * @param 0x7C SPI Flash
 * @param 0x25 ADC Temperature Sensors
 * 
 * @return 0x00 Installed;
 * @return 0x01 Not Installed;
 * @return 0x02 Active;
 * @return 0x03 Inactive;
 * @return UNKNWON if parameter is undefined
 */
void i2c_get_system_check_result(uint32_t parameter){
    //see onenotes of implementation suggestion
}

/**
 * @brief OpCode 0x07 (not implemented)
 * @note Determine time till experiment should be done
 * 
 * @param _unused
 * 
 * @return Time (milli-seconds) till experiment will complete
 */
void i2c_get_time_left_in_experiment(uint32_t parameter){
    //
}

/**
 * @brief Opcode 0x08
 * @note Start experiment task and logging task
 * 
 * @param _unused 
 * 
 * @return VALID if experiment has been started;
 * @return INVALID if experiment was already active
 */
void i2c_start_experiment(uint32_t parameter){
    //check if experiment is already running
    if(payload.status == EXP_INACTIVE) {
        //start logging
        xTaskCreatePinnedToCore(exp_log, "logger", 4096, (void *) &payload.sample_interval, 3, &exp_log_task, 1);

        //start experiment
        xTaskCreatePinnedToCore(exp_run, "experiment", 4096, NULL, 2, &exp_run_task, 1); //i2c on core 0
        
        slave.write_one_byte(VALID);
    }
    else{
        //experiment is already running
        slave.write_one_byte(INVALID);
    }
}
/**
 * @brief OpCode 0x29
 * @note Exit experimeent
 * 
 * @param 0x84 Exit experiment immediately
 * @param 0x33 Exit experiment upon completion of current PWM stage.
 * Cooldown period will then begin to log recovery.
 * 
 * @return VALID once experiment has been stopped;
 * @return INVALID if experiment was not active;
 * @return UNKNOWN if undefined parameter
 */
void i2c_stop_experiment(uint32_t parameter){
    if(parameter == 0x84){
        //Delete experiment task if it has been created
        if(payload.status){
            ESP_LOGD(TAG_i2c, "Stopping Experiment");

            //delete experiment task
            vTaskDelete(exp_run_task);
            payload.status = EXP_INACTIVE;
            ESP_LOGI(TAG_i2c, "Experiment Halted");

            //delete logger task
            while(payload.logger_status == true){
                ESP_LOGD(TAG_i2c, "..");
            }
            vTaskDelete(exp_log_task);
            ESP_LOGI(TAG_i2c, "Experiment Log Halted");

            //turn off pwm
            pwm.pausePWM();

            slave.write_one_byte(VALID);
        }
        else{
            //experiment is not active
            slave.write_one_byte(INVALID);
        }

    }
    else if(parameter == 0x33){
        //check if experiment is active
        if(payload.status){
            ESP_LOGD(TAG_i2c, "Raising experiment stop flag");
            //prompt experiment to end after current stage
            payload.stop_flag = true;

            slave.write_one_byte(VALID);
        }
        else{
            //experiment is not active
            slave.write_one_byte(INVALID);
        }
    }
    else { //Invalid Parameter
        ESP_LOGW(TAG_i2c, "Invalid Parameter");
        slave.write_one_byte(UNKNOWN);
    }
}

/**
 * @brief OpCode 0x06
 * @note Check if the experiment is active
 * 
 * @param _unused
 * 
 * @return VALID if experiment is active;
 * @return INVALID if experiment is inactive
 */
void i2c_get_experiment_status(uint32_t parameter){
    if(payload.status){
        //xperiment is active
        slave.write_one_byte(VALID);
    }
    else{
        //experiment is inactive
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x16
 * @note Get the current PWM stage that the experiment is on
 * 
 * @param _unused
 * 
 * @return Integer number of stage;
 * @return INVALID if experiment is not active
 */
void i2c_get_current_stage(uint32_t parameter){
    if(payload.status == EXP_ACTIVE){
        slave.write_one_byte(payload.current_stage);
    }
    
    if(payload.status == EXP_INACTIVE){
        slave.write_one_byte(INVALID);
    }

    if(payload.status == EXP_STARTUP){
        slave.write_one_byte(STARTUP_TX);
    }

    if(payload.status == EXP_COOLDOWN){
        slave.write_one_byte(COOLDOWN_TX);
    }
}

/**
 * @brief OpCode 0x2A
 * @note Set the number of stages that the experiment will progress
 * through. Default is 16 stages.
 * 
 * @param uint8_t Number of Stages
 * 
 * @return VALID if value was set;
 * @return INVALID if experiment was active and value was not set
 */
void i2c_set_number_of_stages(uint32_t parameter){
    if(!payload.status){
        payload.set_stage_count(parameter);
        ESP_LOGI(TAG_i2c, "Exp Stage Count set to %i", (int)payload.stage_count);

        payload.generate_pwm_duty_array();
        ESP_LOGI(TAG_i2c, "Exp PWM progression generated");

        slave.write_one_byte(VALID);
    }
    else{
        //do not allow changing while experiment is active
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x8D
 * @note Set the amount of time that the experiment will spend on each
 * PWM stage.
 * 
 * @param uint32_t Time (milli-seconds)
 * 
 * @return VALID if value was set;
 * @return INVALID if experiment was active and value was not set
 */
void i2c_set_stage_length(uint32_t parameter){
    //do not allow changing while experiment is active
    if(!payload.status){
        payload.set_stage_length(parameter);
        ESP_LOGI(TAG_i2c, "Exp All Stage Lengths set to %i ms", (int)payload.length[0]);

        slave.write_one_byte(VALID);
    }
    else{
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x9D
 * @note Set the amount of time that the experiment will spend
 * on a specified stage.
 * 
 * @param Stage
 * @param PWM_Duty_%
 * 
 * @return  
 */
void i2c_set_individual_length(uint32_t parameter){
    //do not allow changing while experiment is active
    if(!payload.status){
        //parse parameter
        uint8_t stage = (parameter >> 24) & 0xFF;
        uint32_t length = parameter & 0xFFFFFF;

        ESP_LOGW(TAG_i2c, "stage: %i, length %i", (int)stage, (int)length);

        //check values
        if(stage >= payload.stage_count){
            slave.write_one_byte(0xFD);
        }
        else{
            //set value
            payload.length[stage] = length;
            ESP_LOGI(TAG_i2c, "Exp PWM at Stage #%u set to %lu%%", stage, payload.length[stage]);

            slave.write_one_byte(VALID);
        }
    }
    else{
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x98
 * @note Set the amount of time the experiment will spend before
 * starting the first stage of the experiment. The PWM output will
 * be turned off and the log task will be recording temperature
 * data. Useful to get baseline temperatures before experiment.
 * 
 * @param uint32_t Time (milli-seconds)
 * 
 * @return VALID if value was set;
 * @return INVALID if experiment was active and value was not set 
 */
void i2c_set_startup_length(uint32_t parameter){
    //do not allow changing while experiment is active
    if(!payload.status){
        payload.startup_length = parameter;
        ESP_LOGI(TAG_i2c, "Exp Startup Length set to %i ms", (int)payload.startup_length);

        slave.write_one_byte(VALID);
    }
    else{
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x99
 * @note Set the amount of time the experiment will spend after
 * completing the last stage of the experiment. The PWM output
 * will be turned off and the log will be recording temperature
 * data. Useful to observe how the temperatures normalize.
 * 
 * @param uint32_t Time (milli-seconds)
 * 
 * @return VALID if value was set;
 * @return INVALID if experimnent was active and value was not set
 */
void i2c_set_cooldown_length(uint32_t parameter){
    //do not allow changing while experiment is active
    if(!payload.status){
        payload.cooldown_length = parameter;
        ESP_LOGI(TAG_i2c, "Exp Cooldown Length set to %i ms", (int)payload.cooldown_length);

        slave.write_one_byte(VALID);
    }
    else{
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x5A
 * @note Set the PWM Duty % of a specific stage
 * 
 * @param Stage
 * @param PWM_Duty_%
 * 
 * @return  
 */
void i2c_set_individual_pwm(uint32_t parameter){
    //do not allow changing while experiment is active
    if(!payload.status){
        //parse parameter
        uint8_t stage = (parameter >> 8) & 0xFF;
        uint8_t pwm = parameter & 0xFF;

        //check values
        if(stage >= payload.stage_count){
            slave.write_one_byte(0xFD);
        }
        else if(pwm > 100){
            slave.write_one_byte(0xFE);
        }
        else{
            //set value
            payload.length[stage] = pwm;
            ESP_LOGI(TAG_i2c, "Exp PWM at Stage #%i set to %i%%", (int)stage, (int)payload.length[stage]);

            slave.write_one_byte(VALID);
        }
       
    }
    else{
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x9B (untested)
 * @note Set the PWM output signal's period attribute. System
 * intended for periods greater than one second but can handle
 * values less than one second as well.
 * 
 * @param float Time (seconds)
 * 
 * @return VALID if value was set
 * @return INVALID if experiment was active and value was not set 
 */
void i2c_set_pwm_period(uint32_t parameter){
    //do not allow changing while experiment is active
    if(!payload.status){
        //convert raw hex to float
        union {
            float float_data;
            uint32_t uint_data;
        } float_to_uint = { .uint_data = parameter };

        //check float value
        if(float_to_uint.float_data < PWM_PERIOD_MIN) {
            slave.write_one_byte(0xFD);
        }
        else{
            payload.pwm_period = float_to_uint.float_data;
            ESP_LOGI(TAG_i2c, "Exp PWM Period set to %f s", (float)payload.pwm_period);

            slave.write_one_byte(VALID);
        }

    }
    else{
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x4E (untested)
 * @note Set the temperature threshold to trigger safe mode
 * 
 * @param float Temperature (kelvin) 
 * 
 * @return VALID upon setting value
 */
void i2c_set_max_temperature(uint32_t parameter){
    //do not allow changing while experiment is active
    if(!payload.status){
        //convert raw hex to float
        union {
            float float_data;
            uint32_t uint_data;
        } float_to_uint = { .uint_data = parameter };

        payload.max_temperature = float_to_uint.float_data;
        ESP_LOGI(TAG_i2c, "Exp Max Temperature set to %f s", (float)payload.max_temperature);

        slave.write_one_byte(VALID);
    }
    else{
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x97
 * @note Set the amount of time that the log task will pass in
 * between samples. A lower number means more frequent samples
 * and a higher number means less frequent samples.
 * 
 * @param uint32_t Time (milli-seconds)
 * 
 * @return VALID if value was set
 * @return INVALID if experiment was active and value was not set 
 */
void i2c_set_sampling_interval(uint32_t parameter){
    //do not allow changing while experiment is active
    if(!payload.status){
        payload.sample_interval = parameter;
        ESP_LOGI(TAG_i2c, "Exp Sampling Interval set to %i ms", (int)payload.sample_interval);

        slave.write_one_byte(VALID);
    }
    else{
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x9E
 * @note Set the amount of time that the passivelog task
 * will pass in between samples. A lower number means more
 * frequent samples and a higher number means less frequent
 * samples.
 * 
 * @param uint32_t Time (milli-seconds)
 * 
 * @return VALID if value was set
 * @return INVALID if task is active and value was not set 
 */
void i2c_set_passive_sampling_interval(uint32_t parameter){
    //do not allow changing while task is active
    if(!payload.passive_logger_status){
        payload.sample_passive_interval = parameter;
        ESP_LOGI(TAG_i2c, "Passive Sampling Interval set to %i ms", (int)payload.sample_passive_interval);

        slave.write_one_byte(VALID);
    }
    else{
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x0F (deprecated)
 * @note Prepares experiment log file to be read by the
 * I2C system. This function must be called before get_log
 * 
 * @param _unused
 * 
 * @return VALID when log is ready to be read :)
 */
void i2c_prepare_log(uint32_t parameter){
}

/**
 * @brief OpCode 0x11
 * @note Returns an individual line from the experiment
 * log file. This function must be called after prepare_log.
 * The first call will return the first line, the second
 * call will return the second line, etc... If there is no
 * line to send, it will close the file.
 * 
 * @param _unused
 * 
 * @return string containing one line of log data
 * @return INVALID if there is no log data to return
 */
void i2c_get_log(uint32_t parameter){
    string buffer;
    int line;

    line = storage.readLine(LOG_FILE_NAME,&buffer);
    if(line != -1) {
        slave.write_string(buffer);
    }
    else {
        slave.write_one_byte(INVALID);
    }
}

/**
 * @brief OpCode 0x1C
 * @note Deletes the experiment log and creates a blank
 * log.
 * 
 * @param _unused
 * 
 * @return VALID upon log reset 
 * @return INVALID if SPI error
 */
void i2c_reset_log(uint32_t parameter){
    //erase log
    storage.clearLog(LOG_FILE_NAME);

    //add header
    char header[BUFFER_SIZE];
    active.headerCSV(header);
    storage.addLine(LOG_FILE_NAME, header);

    slave.write_one_byte(VALID);
}

/**
 * @brief OpCode 0x3F
 * @note functions related to passive logger task
 * 
 * @param 0x01 Start Task
 * @param 0x02 Stop Task
 * @param 0x03 Get Status of Task
 * 
 * @return VALID if ask started or stopped successfully
 * @return INVALID if ask already started or stopped
 * @return ACTIVE if task is active
 * @return INACTIVE if task is not active
 * @return UNKNOWN if undefined parameter
 */
void i2c_passive_logger(uint32_t parameter){
    if(parameter == 0x01) { //start logger
        //check if passive logger is already running
        if(payload.passive_logger_status == false) {
            //start task
            xTaskCreatePinnedToCore(exp_log, "plogger", 4096, (void *) &payload.sample_passive_interval, 1, &exp_plog_task, 1);
            payload.passive_logger_status = true;
            ESP_LOGI(TAG_i2c, "Passive Log Task started");

            slave.write_one_byte(VALID);
        }
        else{
            //passive logger is already running
            slave.write_one_byte(INVALID);
        }
    }
    else if(parameter == 0x02) { //stop logger
        if(payload.passive_logger_status == true) {

            vTaskDelete(exp_plog_task);
            payload.passive_logger_status = false;
            ESP_LOGI(TAG_i2c, "Passive Log Task Deleted");

            slave.write_one_byte(VALID);
        }
        else{
            //passive logger is not running
            slave.write_one_byte(INVALID);
        }
    }
    else if(parameter == 0x03) { //return status of logger
        if(payload.passive_logger_status){
            //task is active
            slave.write_one_byte(VALID);
        }
        else{
            //task is inactive
            slave.write_one_byte(INVALID);
        }
    }
    else { //Invalid Parameter
        ESP_LOGW(TAG_i2c, "Invalid Parameter");
        slave.write_one_byte(UNKNOWN);
    }
}

/**
 * @brief OpCode 0x32
 * @note Returns current time of the device from real time 
 * clock object.
 * 
 * @param 0xEE Get Epoch
 * @param 0x4D Get Milli-Second
 * 
 * @return Time (Epoch) for 0xEE;
 * @return Time (milli-seconds) for 0x4D;
 * @return UNKNOWN if undefined parameter
 */
void i2c_get_time(uint32_t parameter){
    if(parameter == 0xEE) { //Epoch
        struct timeval tv;
        gettimeofday(&tv, NULL);

        slave.write_four_bytes(tv.tv_sec);
    }

    else if (parameter == 0x4D) { //us
        struct timeval tv;
        gettimeofday(&tv, NULL);

        slave.write_four_bytes(tv.tv_usec);
    }
    else { //Invalid Parameter
        ESP_LOGW(TAG_i2c, "Invalid Parameter");
        slave.write_one_byte(UNKNOWN);
    }
}

/**
 * @brief OpCode 0x93
 * @note Sets the current time of the device
 * 
 * @param uint32_t Epoch
 * 
 * @return VALID upon setting value
 */
void i2c_set_time(uint32_t parameter){
    ESP_LOGI(TAG_i2c, "System time sync");

    struct timeval tv;
    if (parameter > 2082758399){
        tv.tv_sec = parameter - 2082758399;  // epoch time (seconds)
    } else {
        tv.tv_sec = parameter;  // epoch time (seconds)
    }

    tv.tv_usec = 0;    // microseconds
    settimeofday(&tv, NULL);

    ESP_LOGD(TAG_i2c, "System time set to %x", (int)tv.tv_sec);
    slave.write_one_byte(VALID);
}

/**
 * @brief Opcode 0x34
 * @note Returns the temperature of a specified sensor
 * 
 * @param 0x00 Sensor 0
 * @param 0x01 Sensor 1
 * @param ....
 * @param 0x0F Sensor 15
 * 
 * @return float Temperature
 * @return UNKNOWN if the parameter is undefined
 */
void i2c_get_temperature(uint32_t parameter){
    ESP_LOGW(TAG_i2c, "PARAMETER: %02X", (int)parameter);
    if(parameter >= number_of_temp_sensors) {
        slave.write_one_byte(UNKNOWN);
        ESP_LOGD(TAG_i2c, "%i vs %i", (int)parameter, (int)number_of_temp_sensors);

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
}

/**
 * @brief OpCode 0x15
 * @note Returns the current PWM Duty value
 * 
 * @param _unused
 * 
 * @return int PWM duty (%)
 */
void i2c_get_pwm_duty(uint32_t parameter){
    slave.write_one_byte(pwm.getDutyCycle());
}

extern "C" void app_main(void)
{
    set_system_time_to_compile();

    //log system time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ESP_LOGI(TAG, "System time is %llu/%lu", tv.tv_sec, tv.tv_usec);

    // i2c setup
    slave.init();

    // PWM setup
    pwm.initPWM();

    // SPI Flash reset log
    // deleteLog(&storage);
    // createLog(&storage, &active);

    //define i2c handler call functions
    slave.install_handler_unused(i2c_unused);
    slave.install_handler_ignore(i2c_ignore);
    
    slave.install_handler(0x21, i2c_restart_device);
    slave.install_handler(0x02, i2c_sleep_device);
    slave.install_handler(0x03);
    slave.install_handler(0x06, i2c_get_experiment_status);
    slave.install_handler(0x08, i2c_start_experiment);
    slave.install_handler(0x29, i2c_stop_experiment);
    slave.install_handler(0x2A, i2c_set_number_of_stages);
    slave.install_handler(0x8D, i2c_set_stage_length);
    slave.install_handler(0x4E, i2c_set_max_temperature);
    slave.install_handler(0x0F, i2c_prepare_log);
    slave.install_handler(0x11, i2c_get_log);
    slave.install_handler(0x32, i2c_get_time);
    slave.install_handler(0x93, i2c_set_time);
    slave.install_handler(0x34, i2c_get_temperature);
    slave.install_handler(0x15, i2c_get_pwm_duty);
    slave.install_handler(0x16, i2c_get_current_stage);
    slave.install_handler(0x97, i2c_set_sampling_interval);
    slave.install_handler(0x98, i2c_set_startup_length);
    slave.install_handler(0x99, i2c_set_cooldown_length);
    slave.install_handler(0x5A, i2c_set_individual_pwm);
    slave.install_handler(0x9B, i2c_set_pwm_period);
    slave.install_handler(0x1C, i2c_reset_log);
    slave.install_handler(0x9D, i2c_set_individual_length);
    slave.install_handler(0x9E, i2c_set_passive_sampling_interval);
    slave.install_handler(0x3F, i2c_passive_logger);

    ESP_LOGI(TAG, "Setup completed.");

    xTaskCreatePinnedToCore(i2c_scan, "SCAN", 4096, NULL, tskIDLE_PRIORITY, NULL, 0); //i2c on core 0
}