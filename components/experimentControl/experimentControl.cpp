/**
 * @file Experiment.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of Experiment structure
**/

#include "experimentControl.h"

Experiment::Experiment(){
    status = false;
    logger_status = false;
    passive_logger_status = false;
    pwm_duty = new uint8_t[STAGE_COUNT_MAX];
    length = new uint32_t[STAGE_COUNT_MAX];

    stage_count = 10;
    pwm_period = 12; //12 seconds default
    sample_interval = sec_to_ms(3); //10 second default
    sample_passive_interval = sec_to_ms(9); //1 minute default
    startup_length = min_to_ms(10); //10 minute default
    cooldown_length = min_to_ms(30); //30 minute default
    set_stage_length(min_to_ms(45)); //45 minute default

    generate_pwm_duty_array();
}

Experiment::~Experiment(){
    delete[] pwm_duty;
    delete[] length;
}

exp_err_t Experiment::set_stage_count(uint8_t stage_count_value){
    if(stage_count_value > STAGE_COUNT_MAX) {
        return EXP_BAD_STAGE;
    }

    stage_count = stage_count_value;

    return EXP_OK;
}

exp_err_t Experiment::set_stage_pwm_duty(uint8_t stage, uint8_t pwm_duty_value){
    if(stage > stage_count) {
        return EXP_BAD_STAGE;
    }

    if(pwm_duty_value > 100) {
        return EXP_BAD_PWM;
    }

    pwm_duty[stage] = pwm_duty_value;
    return EXP_OK;
}

exp_err_t Experiment::set_stage_length(uint32_t length_value){
    for(int i = 0; i < STAGE_COUNT_MAX; i++){
        length[i] = length_value;
    }

    return EXP_OK;
}


exp_err_t Experiment::set_max_temperature(float temperature_value){
    max_temperature = temperature_value;
    return EXP_OK;
}

exp_err_t Experiment::generate_pwm_duty_array(){
    for(int i = 0; i < stage_count; i++){
        pwm_duty[i] = ((float)100/stage_count)*(i+1);;
    }

    return EXP_OK;
}