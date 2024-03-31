//
#include "common/common.h"
#include "common/hash_functions.h"
#include "common/pins.h"
#include "common/settings.h"
#include "neurons_list.h"
//
#include "mqtt/mqtt_func.h"
#include "wireless/wireless.h"
#include "servo/servo.h"
//
#include "debug_tools.h"
#include "utility/connectome.h"
#include "worm.h"
//
#include "sdkconfig.h"
//
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"

//
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
//
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


// Factor for converting muscle weights to motor speeds
#define MUSCLE_FACTOR (0.67f)

// Get muscle state every 1 second
#define MOVE_PROCESS_TIMEOUT (1 * 1000 * 1000)


#define STACK_WORDS_SIZE_FOR_TASK_MQTT_PUB (2048)
#define PRIORITY_LEVEL_FOR_TASK_MQTT_PUB   (3)
#define PINNED_CORE_FOR_TASK_MQTT_PUB      (1)
const char* assigned_name_for_task_mqtt_pub = "mqtt_pub";
TaskHandle_t xMQTTPubTaskHandler            = NULL;
StaticTask_t xMQTTPubTaskControlBlock;
StackType_t xMQTTPubStack[STACK_WORDS_SIZE_FOR_TASK_MQTT_PUB];


#define STACK_WORDS_SIZE_FOR_TASK_PETTY_BRAIN (2048)
#define PRIORITY_LEVEL_FOR_TASK_PETTY_BRAIN   (2)
#define PINNED_CORE_FOR_TASK_PETTY_BRAIN      (0)
const char* assigned_name_for_task_petty_brain = "pettie_brain";
TaskHandle_t xPettyBrainTaskHandler            = NULL;
StaticTask_t xPettyBrainTaskControlBlock;
StackType_t xPettyBrainStack[STACK_WORDS_SIZE_FOR_TASK_PETTY_BRAIN];

SemaphoreHandle_t xConnectomeMutexHandler = NULL;
StaticSemaphore_t xConnectomeMutexControlBlock;


#define MSG_BUFFER_SIZE (64)
char mqtt_msg[MSG_BUFFER_SIZE];

#define TOPIC_BUFFER_SIZE (128)
char mqtt_topic[TOPIC_BUFFER_SIZE];



// ----------------------------------------------------------------------
// Static functions

static bool
nose_hit_obstacle(void)
{
  bool res = false;

  // TODO: rewrite this one to the sonar/lidar
  res = gpio_get_level(GENTLE_TOUCH_TAIL_PIN);

  return res;
}

#if 0
static bool
get_tail_body_touch(void)
{
  bool res = false;
  res = gpio_get_level(GENTLE_TOUCH_TAIL_PIN);

  return res;
}
#endif

#if 0
static bool
get_mid_body_touch(void)
{
  bool res = false;
  res = gpio_get_level(GENTLE_TOUCH_MID_PIN);

  return res;
}
#endif


static void
petty_process_move(void)
{
    int16_t speed_l = worm_getLeftMuscle() * MUSCLE_FACTOR;
    int16_t speed_r = worm_getRightMuscle() * MUSCLE_FACTOR;

    int8_t dir_l = 0;
    int8_t dir_r = 0;

    if (speed_l != 0) {
      dir_l = speed_l / abs(speed_l);
    }

    if (speed_r != 0) {
      dir_r = speed_r / abs(speed_r);
    }
    

    if (dir_l == dir_r) {
        if (dir_l > 0) {
            servo_add_action(SERVO_ACT_FORWARD);
        } else {
            servo_add_action(SERVO_ACT_BACKWARD);
        }
    } else {
        if (dir_l > dir_r) {
            servo_add_action(SERVO_ACT_TURN_LEFT);
        } else {
            servo_add_action(SERVO_ACT_TURN_RIGHT);
        }
    }
}

// ----------------------------------------------------------------------
// Accessors functions

void connectome_lock(void)
{
    xSemaphoreTake(xConnectomeMutexHandler, portMAX_DELAY);
}

void connectome_unlock(void)
{
    xSemaphoreGive(xConnectomeMutexHandler);
}


// ----------------------------------------------------------------------
// FreeRTOS functions

static void __attribute__((optimize("-O2")))
mqtt_pub_task(void* pvArg)
{
    (void)pvArg;

    for (;;) {
        connectome_lock();

        Connectome *ctm = worm_getConnectome();
        int16_t cell_weight = 0;

        for (size_t cell_id=0; cell_id < CELLS; cell_id++)
        {
          cell_weight = ctm_get_weight(ctm, cell_id);

          snprintf(mqtt_topic, TOPIC_BUFFER_SIZE, "Pettie/Cells/%s", neurons[cell_id]);
          snprintf(mqtt_msg, MSG_BUFFER_SIZE, "%d", cell_weight);

          mqtt_publish(mqtt_topic, mqtt_msg);
        }

        connectome_unlock();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void __attribute__((optimize("-O2")))
petty_brain_task(void* pvArg)
{
    (void)pvArg;

    int64_t last_time = 0;

    for (;;) {
        connectome_lock();

        if (!nose_hit_obstacle()) {
            worm_chemotaxis();
        } else {
            worm_noseTouch();
        }

        // if (get_tail_body_touch()) {
          // worm_tailBodyTouch();
        // }

        // if (get_mid_body_touch()) {
          // worm_midBodyTouch();
        // }

        connectome_unlock();

        if ((system_get_us_time() - last_time) >= MOVE_PROCESS_TIMEOUT) {
            petty_process_move();

            last_time = system_get_us_time();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL);
}

// ----------------------------------------------------------------------
// Core functions

static void
init_fw_rtos(void)
{
    xConnectomeMutexHandler = xSemaphoreCreateMutexStatic(&xConnectomeMutexControlBlock);
    assert(xConnectomeMutexHandler);



    xPettyBrainTaskHandler = xTaskCreateStaticPinnedToCore((TaskFunction_t)(petty_brain_task),
                                                         assigned_name_for_task_petty_brain,
                                                         STACK_WORDS_SIZE_FOR_TASK_PETTY_BRAIN,
                                                         NULL,
                                                         PRIORITY_LEVEL_FOR_TASK_PETTY_BRAIN,
                                                         xPettyBrainStack,
                                                         &xPettyBrainTaskControlBlock,
                                                         (BaseType_t)PINNED_CORE_FOR_TASK_PETTY_BRAIN);
    assert(xPettyBrainTaskHandler);


    xMQTTPubTaskHandler = xTaskCreateStaticPinnedToCore((TaskFunction_t)(mqtt_pub_task),
                                                         assigned_name_for_task_mqtt_pub,
                                                         STACK_WORDS_SIZE_FOR_TASK_MQTT_PUB,
                                                         NULL,
                                                         PRIORITY_LEVEL_FOR_TASK_MQTT_PUB,
                                                         xMQTTPubStack,
                                                         &xMQTTPubTaskControlBlock,
                                                         (BaseType_t)PINNED_CORE_FOR_TASK_MQTT_PUB);
    assert(xMQTTPubTaskHandler);
}

void init_pettie_core(void)
{
  init_worm();
  init_fw_rtos();
}