#ifndef _SERVO_H
#define _SERVO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


// ----------------------------------------------------------------------
// Definitions, type & enum declaration

typedef struct 
{
    int32_t s5;
    int32_t s6;
    int32_t s7;
    int32_t s8;
    // int32_t s9;
    int32_t s0;
    int32_t s1;
    int32_t s2;
    // int32_t s3;
    int32_t s4;
    int32_t t;
} servo_state_t;

typedef struct {
    int8_t d0;
    int8_t d1;
    int8_t d2;
    // int8_t d3;
    int8_t d4;
    int8_t d5;
    int8_t d6;
    int8_t d7;
    int8_t d8;
} servo_cal_data_t;

typedef enum {
    SERVO_ACT_ZERO = 0,
    SERVO_ACT_CENTER,
    //
    SERVO_ACT_STANDBY,
    SERVO_ACT_FORWARD,
    SERVO_ACT_BACKWARD,
    SERVO_ACT_SHIFT_LEFT,
    SERVO_ACT_SHIFT_RIGHT,
    SERVO_ACT_TURN_LEFT,
    SERVO_ACT_TURN_RIGHT,
    //
    SERVO_ACT_MAX
} servo_action_type_t;


typedef struct {
    servo_action_type_t type;
    servo_state_t *states;
    size_t num_states;
} servo_action_t;




// ----------------------------------------------------------------------
// Variables


// ----------------------------------------------------------------------
// Accessors functions

/**
 * @brief Set servo angle
 * 
 * @param servo_idx Servo number which should me moved
 * @param angle in degrees to where servo must turn
 * 
 * @retval none
 */
void servo_set_angle(int32_t servo_idx, int32_t angle);

/**
 * @brief Send action direction for the servos
 * 
 * @param servo_action index of action to take from @ref servo_actions array
 * 
 * @note this will push item to the queue, and executed later in @ref servo_task
 * 
 * @retval none
 */
void servo_add_action(servo_action_type_t servo_action);

/**
 * @brief This function will store calibration data to the NVS
 * 
 * @param xForcedSave If no storage presented, force to create it and save
 * 
 * @retval none
 * 
 * @note Before calling this function update of the internal data 
 *       must be done with @ref servo_pwm_update_cal_data
 */ 
void servo_pwm_nvs_update_cal_data(bool xForcedSave);

/**
 * @brief This function copies data from memory_model to the internal storage
 * 
 * @retval none
 */
void servo_pwm_update_cal_data(void);

// ----------------------------------------------------------------------
// Core functions

/**
 * @brief Initialise core Servo control functions
 * 
 * @retval none
 */ 
void init_servo(void);

/**
 * @brief Initialise low level LEDC functionality
 * 
 * @retval none
 */
void init_servo_pwm(void);


#ifdef __cplusplus
}
#endif

#endif // _SERVO_H