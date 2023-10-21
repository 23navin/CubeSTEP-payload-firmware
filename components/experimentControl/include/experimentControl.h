/**
 * @file Experiment.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Stores and manages experiment parameters and protocols
**/

#ifndef _Experiment_H_included
#define _Experiment_H_included

namespace experimentControl{
    //experiment parameter
    constexpr int maxStages = 32;

    //types
    typedef unsigned char uint8_t;
    typedef unsigned long uint32_t;
    typedef int exp_err_t;

    //time conversions
    inline uint32_t min_to_ms(uint32_t minute){
        return minute*60000;
    }
    inline uint32_t sec_to_ms(uint32_t second){
        return second*1000;
    }

    //error states
    constexpr int EXP_OK = 0x0;
    constexpr int EXP_BAD_STAGE = 0x101;
    constexpr int EXP_BAD_PWM = 0x102;

    //experiment states
    constexpr int EXP_INACTIVE = 0;
    constexpr int EXP_ACTIVE = 1;
    constexpr int EXP_STARTUP = 2;
    constexpr int EXP_COOLDOWN = 3;

    /**
     * @brief Experiment parameter structure
     * @note - Holds all parameters and settings needed to run an experiment
     * @note - Methods to adjust and set some parameters correctly
     */
    struct Experiment{
        /* member declarations */

        uint8_t stage_count; //Number of PWM stage in the experiment (max 32 in this implementation)
        uint8_t current_stage; //Number of the stage the experiment is currently in
        uint8_t *pwm_duty; //Array that contains the PWM duty for each stage
        float pwm_period; //Length of PWM signal's period in seconds
        uint32_t *length; //Length of each PWM stage is milli-seconds
        uint32_t sample_interval; //Time (milli-seconds) between each temperature sample during experiment
        uint32_t sample_passive_interval; //Time (milli-seconds) between each temperature sample for passive log
        uint32_t startup_length; //Length of time before experiment starts in milli-seconds
        uint32_t cooldown_length; //Length of time after experiment ends before task ends in milli-seconds

        float max_temperature; //Temperature threshold to trigger safe mode
        int status; //Indicates what stage the  experiment task is in.
        bool stop_flag; //If set to true, active experiment will exit once current PWM stage is completed
        bool logger_status; //if true, logger is active
        bool passive_logger_status; //if true, passive logger is active

        /* methods */

        /**
         * @brief Construct a new Experiment object
         * @note Sets experiment parameters to default values
         * 
         */
        Experiment();
        ~Experiment();

        /**
         * @brief Set how many stages the experiment will consists of
         * 
         * @param stage_count_value 
         * @return exp_err_t 
         */
        exp_err_t set_stage_count(uint8_t stage_count_value);

        /**
         * @brief Set PWM duty for a specified stage
         * 
         * @param stage 
         * @param pwm_duty_value 
         * @return exp_err_t 
         */
        exp_err_t set_stage_pwm_duty(uint8_t stage, uint8_t pwm_duty_value);

        /**
         * @brief Set the length of each stage
         * 
         * @param length_value milli-seconds
         * @return exp_err_t 
         */
        exp_err_t set_stage_length(uint32_t length_value);

        /**
         * @brief Set the max temperature
         * 
         * @param temperature_value kelvin
         * @return exp_err_t 
         */
        exp_err_t set_max_temperature(float temperature_value);

        /**
         * @brief Assign a linear progression to PWM Duty array
         * 
         * @return exp_err_t 
         */
        exp_err_t generate_pwm_duty_array();
    };
}

#endif // _Experiment_H_included