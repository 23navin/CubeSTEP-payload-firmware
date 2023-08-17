/**
 * @file Heater.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of HeaterCore class
 * 
 */

#include "Heater.h"

//Callback function that raises associated trigger when timer reaches alarm
bool IRAM_ATTR timer_isr_callback(void *args) {
    bool *trigger = (bool *) args;
    *trigger = true;

    return true; //unsure if something is missing here
}

HeaterCore::HeaterCore(){
    //PWM output
    initGPIO();
    
    //Fill Timer Config
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
    //Initiate Timers
    configureTimers(TIMER_GROUP, TIMER_ID_MAIN, cyclePeriod, &triggerCycle);
    configureTimers(TIMER_GROUP, TIMER_ID_ON, dutyPeriod, &triggerDuty);
}

void HeaterCore::startPWM(){
    //Making sure triggers are off
    triggerCycle = false;
    triggerDuty = false;

    //turn on pwm output initially
    gpio_set_level(PWM_OUTPUT_PIN, HIGH);

    //Start timers
    resumePWM();
}

void HeaterCore::resetPWM(){
    //pause timers
    if(statusTimer == true){
        pausePWM();
    }

    //reset counter values
    timer_set_counter_value(TIMER_GROUP, TIMER_ID_MAIN, 0);
    timer_set_counter_value(TIMER_GROUP, TIMER_ID_ON, 0);

    //manually turn off gpio output
    gpio_set_level(PWM_OUTPUT_PIN, LOW);
}

void HeaterCore::pausePWM(){
    timer_pause(TIMER_GROUP, TIMER_ID_MAIN);
    timer_pause(TIMER_GROUP, TIMER_ID_ON);
    statusTimer = false;
    gpio_set_level(PWM_OUTPUT_PIN, LOW);
}

void HeaterCore::resumePWM(){
    timer_start(TIMER_GROUP, TIMER_ID_MAIN);
    timer_start(TIMER_GROUP, TIMER_ID_ON);
    statusTimer = true;
    gpio_set_level(PWM_OUTPUT_PIN, HIGH);

}

bool HeaterCore::pwmAction(bool instantAction){
    bool trigger = false;

    //Period Timer
    if(triggerCycle == true){
        timer_start(TIMER_GROUP, TIMER_ID_ON); //start duty timer
        triggerCycle = false; //reset trigger
        gpio_set_level(PWM_OUTPUT_PIN, HIGH); //output high at beginning of cycle

        trigger = true;
    }

    //Duty Timer
    if(triggerDuty == true){
        timer_pause(TIMER_GROUP, TIMER_ID_ON); //stop duty timer
        triggerDuty = false; //reset trigger
        gpio_set_level(PWM_OUTPUT_PIN, LOW); //output low at end of duty

        if(instantAction) trigger = true;
    }

    return trigger;
}

void HeaterCore::setPWM(float cycle_period, float duty_period){
    bool buffer = statusTimer; //holds statusTimer=true while timer is paused

    //temporarily pause PWM timers if running
    if(buffer == true){
        pausePWM();
        statusTimer = false;
    }

    //set PWM cycle and duty periods
    cyclePeriod = cycle_period;
    dutyPeriod = duty_period;

    timer_set_alarm_value(TIMER_GROUP, TIMER_ID_MAIN, cyclePeriod*TIMER_SCALE);
    timer_set_alarm_value(TIMER_GROUP, TIMER_ID_ON, dutyPeriod*TIMER_SCALE);

    //unpause
    if(buffer == true){
        resumePWM();
        statusTimer = true;
    }
}

void HeaterCore::setDuty(float duty_period){
    bool buffer = statusTimer; //holds statusTimer=true while timer is paused

    //temporarily pause PWM timers if running
    if(buffer == true){
        pausePWM();
        statusTimer = false;
    }

    //set duty period
    dutyPeriod = duty_period;
    timer_set_alarm_value(TIMER_GROUP, TIMER_ID_ON, dutyPeriod*TIMER_SCALE);

    //unpause
    if(buffer == true){
        resumePWM();
        statusTimer = true;
    }

}

float HeaterCore::getCyclePeriod(){
    return cyclePeriod;
}

float HeaterCore::getDutyPeriod(){
    return dutyPeriod;
}

void HeaterCore::configureTimers(timer_group_t timer_group, timer_idx_t timer_id, float alarm_value, bool *trigger){
    timer_init(timer_group, timer_id, &tmr_config);
    timer_set_counter_value(timer_group, timer_id, 0);
    timer_set_alarm_value(timer_group, timer_id, (alarm_value)*TIMER_SCALE);
    timer_enable_intr(timer_group, timer_id);
    timer_isr_callback_add(timer_group, timer_id, timer_isr_callback, trigger, 0);
    // timer_start(timer_group, timer_id);
}