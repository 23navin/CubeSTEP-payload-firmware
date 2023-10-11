/**
 * @file Heater.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of HeaterCore class
 * 
 */

#include "pwmControl.h"
static const char* TAG = "HeaterCore";

bool IRAM_ATTR timer_isr_callback_cycle(void *args) {
    gpio_set_level(PWM_OUTPUT_PIN, LEVEL_HIGH);
    timer_start(TIMER_GROUP, TIMER_ID_ON);

    return true; //unsure if something is missing here
}

bool IRAM_ATTR timer_isr_callback_duty(void *args) {
    gpio_set_level(PWM_OUTPUT_PIN, LEVEL_LOW);
    timer_pause(TIMER_GROUP, TIMER_ID_ON);

    return true; //unsure if something is missing here
}

HeaterCore::HeaterCore(){
    //PWM output
    initGPIO();
    
    //Fill Timer Config; todo: move to header
    tmr_config.alarm_en = TIMER_ALARM_EN;
    tmr_config.counter_en = TIMER_PAUSE;
    tmr_config.counter_dir = TIMER_COUNT_UP;
    tmr_config.auto_reload = TIMER_AUTORELOAD_EN;
    tmr_config.divider = TIMER_DIVIDER;
}

HeaterCore::~HeaterCore(){ 
    //De-initiate timers
    timer_deinit(TIMER_GROUP, TIMER_ID_MAIN);
    timer_deinit(TIMER_GROUP, TIMER_ID_ON);
}

void HeaterCore::initGPIO(){
    //Set up config object
    gpio_config_t io_conf = {
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    //Configure LED using config object
    gpio_config(&io_conf);
}

void HeaterCore::initPWM(){
    //Cycle timer
    timer_init(TIMER_GROUP, TIMER_ID_MAIN, &tmr_config);
    timer_set_counter_value(TIMER_GROUP, TIMER_ID_MAIN, 0);
    timer_set_alarm_value(TIMER_GROUP, TIMER_ID_MAIN, (cyclePeriod)*TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP, TIMER_ID_MAIN);
    timer_isr_callback_add(TIMER_GROUP, TIMER_ID_MAIN, timer_isr_callback_cycle, NULL, 0);

    //Duty timer
    timer_init(TIMER_GROUP, TIMER_ID_ON, &tmr_config);
    timer_set_counter_value(TIMER_GROUP, TIMER_ID_ON, 0);
    timer_set_alarm_value(TIMER_GROUP, TIMER_ID_ON, (dutyPeriod)*TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP, TIMER_ID_ON);
    timer_isr_callback_add(TIMER_GROUP, TIMER_ID_ON, timer_isr_callback_duty, NULL, 0);
}

void HeaterCore::startPWM(){

    //Start timers
    resumePWM();
}

void HeaterCore::resetPWM(){
    ESP_LOGD(TAG, "PWM output reset");
    //pause timers
    if(statusTimer == true){
        pausePWM();
    }

    //reset counter values
    timer_set_counter_value(TIMER_GROUP, TIMER_ID_MAIN, 0);
    timer_set_counter_value(TIMER_GROUP, TIMER_ID_ON, 0);

    //manually turn off gpio output
    gpio_set_level(PWM_OUTPUT_PIN, LEVEL_LOW);
}

void HeaterCore::pausePWM(){
    //pause timers
    timer_pause(TIMER_GROUP, TIMER_ID_MAIN);
    timer_pause(TIMER_GROUP, TIMER_ID_ON);
    statusTimer = false;
    ESP_LOGI(TAG, "PWM output: off");

    //turn off pwm output
    gpio_set_level(PWM_OUTPUT_PIN, LEVEL_LOW);

}

void HeaterCore::resumePWM(){
    //if 100% duty cycle, turn output on without using pwm timers fixes bug where 100% would cause output to toggle when pwm finished a cycle
    if(dutyPeriod == cyclePeriod){
        gpio_set_level(PWM_OUTPUT_PIN, LEVEL_HIGH);
    }
    else{
        //start timers
        timer_start(TIMER_GROUP, TIMER_ID_MAIN);
        timer_start(TIMER_GROUP, TIMER_ID_ON);

        //turn pwm output to high at beginning of cycle
        gpio_set_level(PWM_OUTPUT_PIN, LEVEL_HIGH);
    }
    statusTimer = true;
    ESP_LOGI(TAG, "PWM output: on");
}

void HeaterCore::setPWM(float cycle_period, float duty_period){
    bool buffer = statusTimer; //holds statusTimer=true while timer is paused

    //temporarily pause PWM timers if running
    if(buffer == true){
        pausePWM();
    }

    //ensure new values are within range
    if(cycle_period <= 0) cycle_period = getCyclePeriod();
    if(duty_period > cycle_period) duty_period = cycle_period;
    if(duty_period < 0) duty_period = 0;

    //set PWM cycle and duty periods
    cyclePeriod = cycle_period;
    dutyPeriod = duty_period;
    ESP_LOGI(TAG, "PWM timers changed");
    // ESP_LOGD(TAG, "PWM Cycle Period set to %.2f seconds", cycle_period);
    // ESP_LOGD(TAG, "PWM Duty Period set to %.2f seconds", duty_period);

    timer_set_alarm_value(TIMER_GROUP, TIMER_ID_MAIN, cyclePeriod*TIMER_SCALE);
    timer_set_alarm_value(TIMER_GROUP, TIMER_ID_ON, dutyPeriod*TIMER_SCALE);

    //unpause
    if(buffer == true){
        resumePWM();
    }
}

void HeaterCore::setDutyPeriod(float duty_period){
    setPWM(duty_period, getCyclePeriod());
}

void HeaterCore::setDutyCycle(int duty_cycle, float cycle_period){
    float duty = cycle_period * ((float)duty_cycle / 100);

    setPWM(cycle_period, duty);

}

void HeaterCore::setDutyCycle(int duty_cycle){
    setDutyCycle(duty_cycle, getCyclePeriod());
    // ESP_LOGI(TAG, "PWM Duty Cycle set to %i%", duty_cycle);
}

float HeaterCore::getCyclePeriod(){
    return cyclePeriod;
}

float HeaterCore::getDutyPeriod(){
    return dutyPeriod;
}