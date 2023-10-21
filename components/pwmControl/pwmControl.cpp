/**
 * @file Heater.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of pwm class
 * 
 */

#include "pwmControl.h"
static const char* TAG = "pwm";

static bool IRAM_ATTR timer_isr_callback_cycle(void *args) {
    gpio_set_level(pwmControl::pwm_pin, pwmControl::levelHigh);
    timer_start(pwmControl::timerGroup, pwmControl::timerIdOn);

    return true; //unsure if something is missing here
}

static bool IRAM_ATTR timer_isr_callback_duty(void *args) {
    gpio_set_level(pwmControl::pwm_pin, pwmControl::levelLow);
    timer_pause(pwmControl::timerGroup, pwmControl::timerIdOn);

    return true; //unsure if something is missing here
}

pwmControl::pwm::pwm(gpio_num_t pwm_io_pin){
    //PWM output
    initGPIO();
    
    //Fill Timer Config; todo: move to header
    tmr_config.alarm_en = TIMER_ALARM_EN;
    tmr_config.counter_en = TIMER_PAUSE;
    tmr_config.counter_dir = TIMER_COUNT_UP;
    tmr_config.auto_reload = TIMER_AUTORELOAD_EN;
    tmr_config.divider = timerDivider;
}

pwmControl::pwm::~pwm(){ 
    //De-initiate timers
    timer_deinit(timerGroup, timerIdMain);
    timer_deinit(timerGroup, timerIdOn);
}

void pwmControl::pwm::initGPIO(){
    //Set up config object
    gpio_config_t io_conf = {
        .pin_bit_mask = (uint64_t)0x1 << pwm_pin,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    //Configure LED using config object
    gpio_config(&io_conf);
}

void pwmControl::pwm::initPWM(){
    //Cycle timer
    timer_init(timerGroup, timerIdMain, &tmr_config);
    timer_set_counter_value(timerGroup, timerIdMain, 0);
    timer_set_alarm_value(timerGroup, timerIdMain, (cyclePeriod)*timerScale);
    timer_enable_intr(timerGroup, timerIdMain);
    timer_isr_callback_add(timerGroup, timerIdMain, timer_isr_callback_cycle, NULL, 0);

    //Duty timer
    timer_init(timerGroup, timerIdOn, &tmr_config);
    timer_set_counter_value(timerGroup, timerIdOn, 0);
    timer_set_alarm_value(timerGroup, timerIdOn, (dutyPeriod)*timerScale);
    timer_enable_intr(timerGroup, timerIdOn);
    timer_isr_callback_add(timerGroup, timerIdOn, timer_isr_callback_duty, NULL, 0);
}

void pwmControl::pwm::startPWM(){

    //Start timers
    resumePWM();
}

void pwmControl::pwm::resetPWM(){
    ESP_LOGD(TAG, "PWM output reset");
    //pause timers
    if(statusTimer == true){
        pausePWM();
    }

    //reset counter values
    timer_set_counter_value(timerGroup, timerIdMain, 0);
    timer_set_counter_value(timerGroup, timerIdOn, 0);

    //manually turn off gpio output
    gpio_set_level(pwm_pin, levelLow);
}

void pwmControl::pwm::pausePWM(){
    //pause timers
    timer_pause(timerGroup, timerIdMain);
    timer_pause(timerGroup, timerIdOn);
    statusTimer = false;
    ESP_LOGI(TAG, "PWM output: off");

    //turn off pwm output
    gpio_set_level(pwm_pin, levelLow);

}

void pwmControl::pwm::resumePWM(){
    //if 100% duty cycle, turn output on without using pwm timers fixes bug where 100% would cause output to toggle when pwm finished a cycle
    if(dutyPeriod == cyclePeriod){
        gpio_set_level(pwm_pin, levelHigh);
    }
    else{
        //start timers
        timer_start(timerGroup, timerIdMain);
        timer_start(timerGroup, timerIdOn);

        //turn pwm output to high at beginning of cycle
        gpio_set_level(pwm_pin, levelHigh);
    }
    statusTimer = true;
    ESP_LOGI(TAG, "PWM output: on");
}

void pwmControl::pwm::setPWM(float cycle_period, float duty_period){
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

    timer_set_alarm_value(timerGroup, timerIdMain, cyclePeriod*timerScale);
    timer_set_alarm_value(timerGroup, timerIdOn, dutyPeriod*timerScale);

    //unpause
    if(buffer == true){
        resumePWM();
    }
}

void pwmControl::pwm::setDutyPeriod(float duty_period){
    setPWM(duty_period, getCyclePeriod());
}

void pwmControl::pwm::setDutyCycle(int duty_cycle, float cycle_period){
    float duty = cycle_period * ((float)duty_cycle / 100);

    setPWM(cycle_period, duty);

}

void pwmControl::pwm::setDutyCycle(int duty_cycle){
    setDutyCycle(duty_cycle, getCyclePeriod());
    // ESP_LOGI(TAG, "PWM Duty Cycle set to %i%", duty_cycle);
}

float pwmControl::pwm::getCyclePeriod(){
    return cyclePeriod;
}

float pwmControl::pwm::getDutyPeriod(){
    return dutyPeriod;
}