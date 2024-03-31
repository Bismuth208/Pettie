
#include "servo.h"
#include "common/common.h"
#include "memory_model/memory_model.h"

//
#include "debug_tools.h"

//
#include <sdkconfig.h>
//
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/timers.h>
//
#include <esp_attr.h>
//
#include <assert.h>
#include <string.h>


// ----------------------------------------------------------------------
// Definitions, type & enum declaration

// GPIO / Servo number
//
//            Head
//   ----             ----
// |  S4  |         |  S8 |
//   ----  --------   ----
//       |  S2    S7  |
//   S3* |            | S9*
//       |  S1    S6  |
//   ----  --------   ----
// |  S0  |         |  S5 |
//   ----             ----
//

// Servos matrix
#define ALLMATRIX (9) // S5, S6, S7, S8, S0, S1, S2, S4 + Run Time
#define ALLSERVOS (8) // S5, S6, S7, S8, S0, S1, S2, S4


// Servo delay base time
#define BASEDELAYTIME (10)

// ----------------------------------------------------------------------
// FreeRTOS Variables

#define STACK_WORDS_SIZE_FOR_TASK_SERVO (2048)
#define PRIORITY_LEVEL_FOR_TASK_SERVO   (1)
#define PINNED_CORE_FOR_TASK_SERVO      (1)
const char* assigned_name_for_task_servo = "servo_usr";
TaskHandle_t xServoTaskHandler           = NULL;
StaticTask_t xServoTaskControlBlock;
StackType_t xServoTaskStack[STACK_WORDS_SIZE_FOR_TASK_SERVO];

//
#define SERVO_QUEUE_SIZE (2UL)
QueueHandle_t xServoQueueHandler = NULL;
StaticQueue_t xServoQueueControlBlock;
size_t xServoQueueStorage[SERVO_QUEUE_SIZE];


// ----------------------------------------------------------------------
// Variables

// Motion data index
int servo_current_state;

// Backup servo value
servo_state_t servo_running_pos;


// clang-format on

// Servo zero position
servo_state_t DRAM_ATTR servo_act_zero = { 
    // S5, S6, S7, S8, S0, S1, S2, S4,  ms
    135,  45, 135,  45,  45, 135,  45, 135,  500
};

// Start position
servo_state_t DRAM_ATTR servo_act_center = {
    // S5, S6, S7, S8, S0, S1, S2, S4,  ms
    90,  90,  90,  90,  90,  90,  90,  90,  500
};

// Standby
servo_state_t DRAM_ATTR servo_act_standby[] = {
  // S5, S6, S7, S8, S0, S1, S2, S4,  ms
  {   90,  90,  90,  90,  90,  90,  90,  90,  500  }, // servo center point
  {   70,  90,  90, 110, 110,  90,  90,  70,  500  }, // standby
};

// Forward
servo_state_t DRAM_ATTR servo_act_forward[] = {
  // S5, S6, S7, S8, S0, S1, S2, S4,  ms
  {   90,  90,  90, 110, 110,  90,  45,  90,  200  }, // leg1,4 up; leg4 fw
  {   70,  90,  90, 110, 110,  90,  45,  70,  200  }, // leg1,4 dn
  {   70,  90,  90,  90,  90,  90,  45,  70,  200  }, // leg2,3 up
  {   70,  45, 135,  90,  90,  90,  90,  70,  200  }, // leg1,4 bk; leg2 fw
  {   70,  45, 135, 110, 110,  90,  90,  70,  200  }, // leg2,3 dn
  {   90,  90, 135, 110, 110,  90,  90,  90,  200  }, // leg1,4 up; leg1 fw
  {   90,  90,  90, 110, 110, 135,  90,  90,  200  }, // leg2,3 bk
  {   70,  90,  90, 110, 110, 135,  90,  70,  200  }, // leg1,4 dn
  {   70,  90,  90, 110,  90, 135,  90,  70,  200  }, // leg3 up
  {   70,  90,  90, 110, 110,  90,  90,  70,  200  }, // leg3 fw dn (standby)
};

// Backward
servo_state_t DRAM_ATTR servo_act_backward[] = {
  // S5, S6, S7, S8, S0, S1, S2, S4,  ms
  {   90,  45,  90, 110, 110,  90,  90,  90,  200  }, // leg4,1 up; leg1 fw
  {   70,  45,  90, 110, 110,  90,  90,  70,  200  }, // leg4,1 dn
  {   70,  45,  90,  90,  90,  90,  90,  70,  200  }, // leg3,2 up
  {   70,  90,  90,  90,  90, 135,  45,  70,  200  }, // leg4,1 bk; leg3 fw
  {   70,  90,  90, 110, 110, 135,  45,  70,  200  }, // leg3,2 dn
  {   90,  90,  90, 110, 110, 135,  90,  90,  200  }, // leg4,1 up; leg4 fw
  {   90,  90, 135, 110, 110,  90,  90,  90,  200  }, // leg3,1 bk
  {   70,  90, 135, 110, 110,  90,  90,  70,  200  }, // leg4,1 dn
  {   70,  90, 135,  90, 110,  90,  90,  70,  200  }, // leg2 up
  {   70,  90,  90, 110, 110,  90,  90,  70,  200  }, // leg2 fw dn (standby)
};

// Left shift
servo_state_t DRAM_ATTR servo_act_shift_left[] = {
  // S5, S6, S7, S8, S0, S1, S2, S4,  ms
  {   70,  90,  45,  90,  90,  90,  90,  70,  200  }, // leg3,2 up; leg2 fw
  {   70,  90,  45, 110, 110,  90,  90,  70,  200  }, // leg3,2 dn
  {   90,  90,  45, 110, 110,  90,  90,  90,  200  }, // leg1,4 up
  {   90, 135,  90, 110, 110,  45,  90,  90,  200  }, // leg3,2 bk; leg1 fw
  {   70, 135,  90, 110, 110,  45,  90,  70,  200  }, // leg1,4 dn
  {   70, 135,  90,  90,  90,  90,  90,  70,  200  }, // leg3,2 up; leg3 fw
  {   70,  90,  90,  90,  90,  90, 135,  70,  200  }, // leg1,4 bk
  {   70,  90,  90, 110, 110,  90, 135,  70,  200  }, // leg3,2 dn
  {   70,  90,  90, 110, 110,  90, 135,  90,  200  }, // leg4 up
  {   70,  90,  90, 110, 110,  90,  90,  70,  200  }, // leg4 fw dn (standby)
};

// Right shift
servo_state_t DRAM_ATTR servo_act_shift_right[] = {
  // S5, S6, S7, S8, S0, S1, S2, S4,  ms
  {   70,  90,  90,  90,  90,  45,  90,  70,  200  }, // leg2,3 up; leg3 fw
  {   70,  90,  90, 110, 110,  45,  90,  70,  200  }, // leg2,3 dn
  {   90,  90,  90, 110, 110,  45,  90,  90,  200  }, // leg4,1 up
  {   90,  90,  45, 110, 110,  90, 135,  90,  200  }, // leg2,3 bk; leg4 fw
  {   70,  90,  45, 110, 110,  90, 135,  70,  200  }, // leg4,1 dn
  {   70,  90,  90,  90,  90,  90, 135,  70,  200  }, // leg2,3 up; leg2 fw
  {   70, 135,  90,  90,  90,  90,  90,  70,  200  }, // leg4,1 bk
  {   70, 135,  90, 110, 110,  90,  90,  70,  200  }, // leg2,3 dn
  {   90, 135,  90, 110, 110,  90,  90,  70,  200  }, // leg1 up
  {   70,  90,  90, 110, 110,  90,  90,  70,  200  }, // leg1 fw dn (standby)
};

// Turn left
servo_state_t DRAM_ATTR servo_act_turn_left[] = {
  // S5, S6, S7, S8, S0, S1, S2, S4,  ms
  {   90,  90,  90, 110, 110,  90,  90,  90,  200  }, // leg1,4 up
  {   90, 135,  90, 110, 110,  90, 135,  90,  200  }, // leg1,4 turn
  {   70, 135,  90, 110, 110,  90, 135,  70,  200  }, // leg1,4 dn
  {   70, 135,  90,  90,  90,  90, 135,  70,  200  }, // leg2,3 up
  {   70, 135, 135,  90,  90, 135, 135,  70,  200  }, // leg2,3 turn
  {   70, 135, 135, 110, 110, 135, 135,  70,  200  }, // leg2,3 dn
  {   70,  90,  90, 110, 110,  90,  90,  70,  200  }, // leg1,2,3,4 turn (standby)
};

// Turn right
servo_state_t DRAM_ATTR servo_act_turn_right[] = {
  // S5, S6, S7, S8, S0, S1, S2, S4,  ms
  {   70,  90,  90,  90,  90,  90,  90,  70,  200  }, // leg2,3 up
  {   70,  90,  45,  90,  90,  45,  90,  70,  200  }, // leg2,3 turn
  {   70,  90,  45, 110, 110,  45,  90,  70,  200  }, // leg2,3 dn
  {   90,  90,  45, 110, 110,  45,  90,  90,  200  }, // leg1,4 up
  {   90,  45,  45, 110, 110,  45,  45,  90,  200  }, // leg1,4 turn
  {   70,  45,  45, 110, 110,  45,  45,  70,  200  }, // leg1,4 dn
  {   70,  90,  90, 110, 110,  90,  90,  70,  200  }, // leg1,2,3,4 turn (standby)
};


servo_action_t servo_actions[] =
{
    //
    {SERVO_ACT_ZERO, (servo_state_t*)&servo_act_zero, 1},
    {SERVO_ACT_CENTER, (servo_state_t*)&servo_act_center, 1},
    {SERVO_ACT_STANDBY, (servo_state_t*)&servo_act_standby, 2},
    //
    {SERVO_ACT_FORWARD, (servo_state_t*)&servo_act_forward, 10},
    {SERVO_ACT_BACKWARD, (servo_state_t*)&servo_act_backward, 10},
    {SERVO_ACT_SHIFT_LEFT, (servo_state_t*)&servo_act_shift_left, 10},
    {SERVO_ACT_SHIFT_RIGHT, (servo_state_t*)&servo_act_shift_right, 10},
    {SERVO_ACT_TURN_LEFT, (servo_state_t*)&servo_act_turn_left, 7},
    {SERVO_ACT_TURN_RIGHT, (servo_state_t*)&servo_act_turn_right, 7}
};
_Static_assert((sizeof(servo_actions) / sizeof(servo_actions[0])) == (SERVO_ACT_MAX), "servo_actions size fail!");

// clang-format off

// ----------------------------------------------------------------------
// Static functions declaration

static void servo_validate(void);
static void init_servo_rtos(void);

static void servo_run(servo_state_t *servo_matrix, size_t steps);

static void servo_task(void *arg);


// ----------------------------------------------------------------------
// Static functions

static void
servo_run(servo_state_t *servo_matrix, size_t steps)
{
  int32_t tmp_a = 0u;
  int32_t tmp_b = 0u;
  int32_t tmp_c = 0u;

  for (size_t matrix_step = 0u; matrix_step < steps; matrix_step++) {

    int InterTotalTime = servo_matrix[matrix_step].t;
    int InterDelayCounter = InterTotalTime / BASEDELAYTIME;

    int32_t *servo_ptr = (int32_t *) &servo_running_pos;
    int32_t *servo_matrix_ptr = (int32_t *) &servo_matrix[matrix_step];

    for (size_t inter_step_loop = 0u; inter_step_loop < InterDelayCounter; inter_step_loop++) {
      for (size_t servo_index = 0u; servo_index < ALLSERVOS; servo_index++) {

        tmp_a = servo_ptr[servo_index];
        tmp_b = servo_matrix_ptr[servo_index];

        if (tmp_a == tmp_b) {
          tmp_c = tmp_b;
        } else if (tmp_a > tmp_b) {
          tmp_c =  ul_map_val(BASEDELAYTIME * inter_step_loop, 0, InterTotalTime, 0, tmp_a - tmp_b);
          if (tmp_a - tmp_c >= tmp_b) {
            servo_set_angle(servo_index, tmp_a - tmp_c);
          }
        } else if (tmp_a < tmp_b) {
          tmp_c =  ul_map_val(BASEDELAYTIME * inter_step_loop, 0, InterTotalTime, 0, tmp_b - tmp_a);
          if (tmp_a + tmp_c <= tmp_b) {
            servo_set_angle(servo_index, tmp_a + tmp_c);
          }
        }
      }

      vTaskDelay(pdMS_TO_TICKS(BASEDELAYTIME));
    }

    memcpy(&servo_running_pos, &servo_matrix[matrix_step], sizeof(servo_state_t));
  }
}


static void
servo_validate(void)
{
    for (size_t i = 0u; i < SERVO_ACT_MAX; i++) {
        assert(servo_actions[i].type == (servo_action_type_t)i);
        assert(servo_actions[i].states != NULL);
        assert(servo_actions[i].num_states != 0u);
    }
}

// ----------------------------------------------------------------------
// Accessors functions

void servo_add_action(servo_action_type_t servo_action)
{
    assert(servo_action < SERVO_ACT_MAX);

    xQueueSend(xServoQueueHandler, &servo_action, 0);
}

// ----------------------------------------------------------------------
// FreeRTOS functions

static void __attribute__((optimize("-O2")))
servo_task(void *arg)
{
    (void) arg;

    int32_t servo_action = 0;

    for (;;) {
        if (xQueueReceive(xServoQueueHandler, &servo_action, 250)) {

            ASYNC_PRINTF(CONFIG_SERVO_ACTION_DBG_PRINTOUT, async_print_type_u32, "Action: %u", servo_action);

            servo_run(servo_actions[servo_action].states, servo_actions[servo_action].num_states);
        }

        if (memory_model_get_bool(TOPIC_ID_SERVO_CAL_UPDATE, 10) == true) {
          servo_pwm_update_cal_data();

          memory_model_set_bool(TOPIC_ID_SERVO_CAL_UPDATE, 0);
        }

        if (memory_model_get_bool(TOPIC_ID_SERVO_CAL_SAVE, 10) == true) {
          servo_pwm_nvs_update_cal_data(true);

          memory_model_set_bool(TOPIC_ID_SERVO_CAL_SAVE, 0);
        }

        // vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

// ----------------------------------------------------------------------
// Core functions

static void
init_servo_rtos(void)
{
    xServoQueueHandler = xQueueCreateStatic(SERVO_QUEUE_SIZE, sizeof(size_t), (uint8_t*)&xServoQueueStorage[0], &xServoQueueControlBlock);
    assert(xServoQueueHandler);


    xServoTaskHandler = xTaskCreateStaticPinnedToCore((TaskFunction_t)(servo_task),
                                                     assigned_name_for_task_servo,
                                                     STACK_WORDS_SIZE_FOR_TASK_SERVO,
                                                     NULL,
                                                     PRIORITY_LEVEL_FOR_TASK_SERVO,
                                                     xServoTaskStack,
                                                     &xServoTaskControlBlock,
                                                     (BaseType_t)PINNED_CORE_FOR_TASK_SERVO);
    assert(xServoTaskHandler);
}

void init_servo(void)
{
    servo_validate();

    init_servo_pwm();
    init_servo_rtos();

    servo_add_action(SERVO_ACT_STANDBY);
    vTaskDelay(pdMS_TO_TICKS(5000));
}