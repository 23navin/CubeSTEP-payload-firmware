/**
 * @file Heater.h
 * @author Benjamin Navin (bnjames@cpp.edus)
 * 
 * @brief Control battery heater using Trailing Edge PWM
 * 
*/

#ifndef _heater_H_included
#define _heater_H_included

#include "esp_log.h"
#include "esp_err.h"

#include "driver/timer.h"
#include "driver/gpio.h"

#include "Telemetry.h"

// TIMER
#define TIMER_DIVIDER           65536
#define TIMER_SCALE             (TIMER_BASE_CLK / TIMER_DIVIDER)  // one second
#define TIMER_GROUP             TIMER_GROUP_0
#define TIMER_ID_MAIN           TIMER_0
#define TIMER_ID_ON             TIMER_1

// GPIO output
#define PWM_OUTPUT_PIN          GPIO_NUM_5 // GPIO pin used for PWM output
#define GPIO_OUTPUT_PIN_SEL     (1ULL<<PWM_OUTPUT_PIN)

// Parameter Checks
#define PWM_PERIOD_MIN          (0.1)

//GPIO Levels
#define LEVEL_HIGH                    0x1
#define LEVEL_LOW                     0x0

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
     * @note This function is called in the HeaterCore constructor.
     */
    void initGPIO();

    /**
     * @brief Configure timers to create a PWM signal
     * @note call startPWM after to start output
     */
    void initPWM();
    
    /**
     * @brief Start PWM output
     * @note initPWM must be called before calling this function
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
    void setDutyPeriod(float duty_period);

    /**
     * @brief Set the PWM length and duty cycle
     * 
     * @param duty_cycle Duty Cycle of PWM signal from 0% to 100%
     * @param period Length of the PWM duty (seconds)
     */
    void setDutyCycle(int duty_cycle, float period);

    /**
     * @brief Set the Duty Cycle
     * 
     * @param duty_cycle Duty Cycle of PWM signal from 0% to 100%
     */
    void setDutyCycle(int duty_cycle);

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

    inline int getDutyCycle(){
        return (int)((dutyPeriod/cyclePeriod)*100);
    }

    inline bool getStatus(){
        return statusTimer;
    }

    void snapshot(Telemetry *telemetry_out);


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
     * @brief Keeps track of whether the PWM timers are running or not
     * 
     */
    bool statusTimer = false;

};

#endif // _heater_H_included
