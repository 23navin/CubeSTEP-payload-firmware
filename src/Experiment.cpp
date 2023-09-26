/**
 * @file Experiment.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of Experiment structure
**/

#include "Experiment.h"

Experiment::Experiment(){
    stage_count = 0;
    status = false;
    logger_status = false;
    pwm_duty = new uint8_t[STAGE_COUNT_MAX];
    pwm_period = 12; //12 seconds default
    length = min_to_ms(45); //45 minutes default
    sample_interval = min_to_ms(1); //1 minute default
    startup_length = min_to_ms(10); //10 minute default
    cooldown_length = min_to_ms(30); //30 minute default
}

Experiment::~Experiment(){
    delete[] pwm_duty;
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

exp_err_t Experiment::set_max_temperature(float temperature_value){
    max_temperature = temperature_value;
    return EXP_OK;
}

exp_err_t Experiment::generate_parameters(){
    for(int i = 0; i < stage_count; i++){
        pwm_duty[i] = ((float)100/stage_count)*(i+1);;
    }

    return EXP_OK;
}