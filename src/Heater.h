/**
 * @file Heater.h
 * @author Benjamin Navin (bnjames@cpp.edus)
 * 
 * @brief Control battery heater using Trailing Edge PWM
 * 
*/

#ifndef _heater_H_included
#define _heater_H_included

#include "driver/timer.h"
#include "driver/gpio.h"
#include "ESP32Time.h"

// TIMER
#define TIMER_DIVIDER           65536
#define TIMER_SCALE             (TIMER_BASE_CLK / TIMER_DIVIDER)  // one second
#define TIMER_GROUP             TIMER_GROUP_0
#define TIMER_ID_MAIN           TIMER_0
#define TIMER_ID_ON             TIMER_1

// GPIO output
#define PWM_OUTPUT_PIN          GPIO_NUM_5 // GPIO pin used for PWM output
#define GPIO_OUTPUT_PIN_SEL     (1ULL<<PWM_OUTPUT_PIN)

/**
 * @brief Interface to control PWM output for battery heaters
 * @note - Configure GPIO pad
 * @note - Configure General Purpose Timers to create a PWM signal
 * @note - Route PWM signal to a GPIO pin
 * @note - Suspend and resume PWM signal
 */
class HeaterCore{
public:
    /**
     * @brief Construct a new Heater Core object
     * @note calls initGPIO and sets timer config
     */
    HeaterCore();

    /**
     * @brief Destroy the Heater Core object
     * 
     */
    ~HeaterCore();
    
    /**
     * @brief Configure GPIO pin for PWM output
     * @note use startPWM after calling this to begin counting
     */
    void initGPIO();

    /**
     * @brief Configure timers for PWM functionality
     * @note PWM uses both timers in group 0
     */
    void initPWM();
    
    /**
     * @brief Start PWM output
     *
     */
    void startPWM();

    /**
     * @brief Pauses PWM output and resets timing to the beginning of the PWM cycle
     * 
     */
    void resetPWM();

    /**
     * @brief Pause PWM output
     * 
     */
    void pausePWM();

    /**
     * @brief Resume PWM output
     * 
     */
    void resumePWM();
    
    /**
     * @brief Updates PWM output when called
     * @note Call as frequently as possible for best accuracy
     * 
     * @param instantAction If true, function considers completion of duty and cycle as an action. If false (default), function only considers completion of cycle as an action.
     * @return true if an action has occured since last called 
     * @return false if an action has not occured since last called
     */
    bool pwmAction(bool instantAction = false);

    /**
     * @brief Change the PWM cycle and duty periods
     * @note briefly pauses the PWM timers while changing
     * 
     * @param cycle_period Length of the PWM cycle (seconds)
     * @param duty_period Length of the PWM duty (seconds)
     */
    void setPWM(float cycle_period, float duty_period);

    /**
     * @brief Change the PWM duty period
     * @note briefly pauses the PWM timers while changing
     * 
     * @param duty_period Length of the PWM duty (seconds)
     */
    void setDuty(float duty_period);

    /**
     * @brief Get the Cycle Period object
     * 
     * @return float 
     */
    float getCyclePeriod();

    /**
     * @brief Get the Duty Period object
     * 
     * @return float 
     */
    float getDutyPeriod();

private:
    /**
     * @brief Length of the PWM cyle in seconds
     * @note Default is 12 seconds
     * @note Must be longer than dutyPeriod
     */
    float cyclePeriod = 12;

    /**
     * @brief Length of the duty cyle in seconds
     * @note Default is 2 seconds
     * @note Must be shorter than cyclePeriod
     */
    float dutyPeriod = 2;

    /**
     * @brief Config object for timers
     * @note Is filled upon construction of HeaterCore
     */
    timer_config_t tmr_config;

    /**
     * @brief Trigger value for the PWM's Cycle Period
     * @note When true, indicates that PWM period has been reached and PWM output should be turned on
     */
    bool triggerCycle;

    /**
     * @brief Trigger value for the PWM's Duty Period
     * @note When true, indicates that Duty period has been reahed and PWM output should be turned off
     */
    bool triggerDuty;

    /**
     * @brief Keeps track of whether the PWM timers are running or not
     * 
     */
    bool statusTimer = false;

    /**
     * @brief Configure timers
     * @note Call before starting PWM output
     * 
     * @param timer_group Timer group [default is TIMER_GROUP_0]
     * @param timer_id Timer index, either TIMER_0 (PWM cycle timer) or TIMER_1 (PWM duty timer)
     * @param alarm_value A 64-bit value to set the alarm value
     * @param trigger Address to a boolean that will be set to true when the timer reaches its alarm
     */
    void configureTimers(timer_group_t timer_group, timer_idx_t timer_id, float alarm_value, bool *trigger);
};

#endif // _heater_H_included
