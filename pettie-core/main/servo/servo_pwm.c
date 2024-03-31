
#include "servo.h"
#include "memory_model/memory_model.h"
#include "common/pins.h"

//
#include "debug_tools.h"
//
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include <stdio.h>

// ----------------------------------------------------------------------
// Definitions, type & enum declaration

// MG90S servo PWM pulse traveling
// const int PWMRES_Min = 1; // PWM Resolution 1
// const int PWMRES_Max = 180; // PWM Resolution 180
// const int SERVOMIN = 400; // 400
// const int SERVOMAX = 2400; // 2400


#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (((1 << LEDC_DUTY_RES) / 2) - 1) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FULL_DUTY          ((1 << LEDC_DUTY_RES) - 1)
#define LEDC_FREQUENCY          (50) // Frequency in Hertz. 20ms for the Servo pulse_period is mostly OK.


#define SERVO_MAX_ANGLE (180)
#define SERVO_MIN_WDTH_US (500)
#define SERVO_MAX_WDTH_US (2500)
#define SERVO_FREQ (LEDC_FREQUENCY)

/// Where to store the Keys & MAC
#define STORAGE_NAMESPACE "storage"

/// Name of list pair in NVS
#define SERVO_CAL_DATA_NVS_NAME "servo_cal"

// 
// #define SERVO_ENABLE_FADE (1)


typedef struct 
{
    uint8_t channel;
    uint8_t gpio;
} pwm_servo_t;


// ----------------------------------------------------------------------
// Variables

// Pettie has 4 legs with 2 joints in each
// In total 8 servos is required
pwm_servo_t pwmServos[] = {
    { LEDC_CHANNEL_0, SERVO_D5},
    { LEDC_CHANNEL_1, SERVO_D6},
    { LEDC_CHANNEL_2, SERVO_D7},
    { LEDC_CHANNEL_3, SERVO_D8},
    { LEDC_CHANNEL_4, SERVO_D0},
    { LEDC_CHANNEL_5, SERVO_D1},
    { LEDC_CHANNEL_6, SERVO_D2},
    { LEDC_CHANNEL_7, SERVO_D4}
};


servo_cal_data_t servo_cal_data;



// ----------------------------------------------------------------------
// Static functions

static uint32_t servo_pwm_calculate_duty(float angle)
{
    float angle_us = angle / SERVO_MAX_ANGLE * (SERVO_MAX_WDTH_US - SERVO_MIN_WDTH_US) + SERVO_MIN_WDTH_US;
    // ESP_LOGD(TAG, "angle us: %f", angle_us);
    uint32_t duty = (uint32_t)((float)LEDC_FULL_DUTY * (angle_us) * SERVO_FREQ / (1000000.0f));
    return duty;
}

// static float servo_pwm_calculate_angle( uint32_t duty)
// {
//     float angle_us = (float)duty * 1000000.0f / (float)LEDC_FULL_DUTY / (float)SERVO_FREQ;
//     angle_us -= SERVO_MIN_WDTH_US;
//     angle_us = angle_us < 0.0f ? 0.0f : angle_us;
//     float angle = angle_us * SERVO_MAX_ANGLE / (SERVO_MAX_WDTH_US - SERVO_MIN_WDTH_US);
//     return angle;
// }

static float servo_pwm_get_correction_angle(uint8_t channel)
{
  float res = 0;

  // Get calibration value previously loaded from NVS storage
  int8_t *pcal_data = (int8_t *)&servo_cal_data;
  res = (float)pcal_data[channel];

  return res;
}

static esp_err_t servo_pwm_write_angle(uint8_t channel, float angle)
{
    esp_err_t res = ESP_OK;
 
#if (SERVO_ENABLE_FADE == 1)
    res = ledc_set_fade_with_time(LEDC_MODE,
                    (ledc_channel_t)channel, duty, 50);
    res |= ledc_fade_start(LEDC_MODE,
                    (ledc_channel_t)channel, LEDC_FADE_WAIT_DONE);
#else

    angle += servo_pwm_get_correction_angle(channel);
    uint32_t duty = servo_pwm_calculate_duty(angle);

    res = ledc_set_duty(LEDC_MODE, (ledc_channel_t)channel, duty);
    res |= ledc_update_duty(LEDC_MODE, (ledc_channel_t)channel);
#endif
    
    return res;
}


static void init_servo_ledc(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    

    for (size_t i = 0; i < LEDC_CHANNEL_MAX; i++) {
        ledc_channel.channel = pwmServos[i].channel;
        ledc_channel.gpio_num = pwmServos[i].gpio;

        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    }

#if (SERVO_ENABLE_FADE == 1)
    // Initialize fade service.
    ledc_fade_func_install(0);
#endif
}

static void init_servo_pwm_calibration(void)
{
  servo_pwm_nvs_update_cal_data(false);
}


// ----------------------------------------------------------------------
// Accessors functions

void servo_set_angle(int32_t servo_idx, int32_t angle)
{
    assert(servo_idx < LEDC_CHANNEL_MAX);

//   int NewPWM = iValue + (int8_t)EEPROM.read(iServo);
//   NewPWM = map(NewPWM, PWMRES_Min, PWMRES_Max, SERVOMIN, SERVOMAX);

//   if (iServo >= 7) {
//     GPIO2SERVO.write(NewPWM);
//   } else if (iServo >= 6) {
//     GPIO4SERVO.write(NewPWM);
//   } else if (iServo >= 5) {
//     GPIO5SERVO.write(NewPWM);
//   } else if (iServo >= 4) {
//     GPIO16SERVO.write(NewPWM);
//   } else if (iServo >= 3) {
//     GPIO15SERVO.write(NewPWM);
//   } else if (iServo >= 2) {
//     GPIO13SERVO.write(NewPWM);
//   } else if (iServo >= 1) {
//     GPIO12SERVO.write(NewPWM);
//   } else if (iServo == 0) {
//     GPIO14SERVO.write(NewPWM);
//   }

    servo_pwm_write_angle(pwmServos[servo_idx].channel, angle);
}


void
servo_pwm_nvs_update_cal_data(bool xForcedSave)
{
	nvs_handle_t nvs_keys_handle;
	size_t required_size = 0;
	bool xNeedSave = false;

	ESP_ERROR_CHECK(nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_keys_handle));
	esp_err_t err = nvs_get_blob(nvs_keys_handle, SERVO_CAL_DATA_NVS_NAME, NULL, &required_size);

	if(xForcedSave || (required_size != sizeof(servo_cal_data_t))) {
		xNeedSave = true;
    required_size = sizeof(servo_cal_data_t);
	} else {
		// Data been found?
		if(required_size > 0) {
			ESP_ERROR_CHECK(nvs_get_blob(nvs_keys_handle, SERVO_CAL_DATA_NVS_NAME, &servo_cal_data, &required_size));
		} else {
			// No data, so save it
			xNeedSave = true;
		}
	}

	if(xNeedSave == true) {
		ESP_ERROR_CHECK(nvs_set_blob(nvs_keys_handle, SERVO_CAL_DATA_NVS_NAME, &servo_cal_data, required_size));
		ESP_ERROR_CHECK(nvs_commit(nvs_keys_handle));
	}

	nvs_close(nvs_keys_handle);
}


void servo_pwm_update_cal_data(void)
{
  servo_cal_data.d0 = (int8_t)memory_model_get_i32(TOPIC_ID_SERVO_CAL_D0, 100);
  servo_cal_data.d1 = (int8_t)memory_model_get_i32(TOPIC_ID_SERVO_CAL_D1, 100);
  servo_cal_data.d2 = (int8_t)memory_model_get_i32(TOPIC_ID_SERVO_CAL_D2, 100);
  // servo_cal_data.d3 = (int8_t)memory_model_get_i32(TOPIC_ID_SERVO_CAL_D3, 100);
  servo_cal_data.d4 = (int8_t)memory_model_get_i32(TOPIC_ID_SERVO_CAL_D4, 100);
  servo_cal_data.d5 = (int8_t)memory_model_get_i32(TOPIC_ID_SERVO_CAL_D5, 100);
  servo_cal_data.d6 = (int8_t)memory_model_get_i32(TOPIC_ID_SERVO_CAL_D6, 100);
  servo_cal_data.d7 = (int8_t)memory_model_get_i32(TOPIC_ID_SERVO_CAL_D7, 100);
  servo_cal_data.d8 = (int8_t)memory_model_get_i32(TOPIC_ID_SERVO_CAL_D8, 100);
}


// ----------------------------------------------------------------------
// Core functions

void init_servo_pwm(void)
{
    init_servo_ledc();
    init_servo_pwm_calibration();
}