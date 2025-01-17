#include "worm.h"

#include "behaviors.h"


Connectome _connectome;

int _leftMuscle;
int _rightMuscle;
double _motorFireAvg; // Percentage of A-type motor neurons firing


void worm_update(const uint16_t*, int);


void init_worm()
{
  _leftMuscle = 0;
  _rightMuscle = 0;

  _motorFireAvg = 16.0;

  ctm_init(&_connectome);
}

void worm_chemotaxis() {
  worm_update(CHEMOTAXIS, CHEMOTAXIS_LEN);
}

void worm_noseTouch() {
  worm_update(NOSE_TOUCH, NOSE_TOUCH_LEN);
}

// void worm_midBodyTouch() {
//     worm_update(BODY_MID_TOUCH, BODY_MID_TOUCH_LEN);
// }

// void worm_tailBodyTouch() {
//     worm_update(BODY_TAIL_TOUCH, BODY_TAIL_TOUCH_LEN);
// }

int worm_getLeftMuscle() {
  return _leftMuscle;
}

int worm_getRightMuscle() {
  return _rightMuscle;
}

Connectome *worm_getConnectome() {
  return &_connectome;
}

void worm_update(const uint16_t* stim_neuron, int len_stim_neuron) {

  Connectome* ctm = &_connectome;

  //
  // Run one tick of neural emulation
  //

  ctm_neural_cycle(ctm, stim_neuron, len_stim_neuron);

  //
  // Aggregate muscle states
  //

  uint16_t body_total = 0;
  // Gather totals on body muscles
  for(int i = 0; i < BODY_MUSCLES; i++) {
    uint16_t left_id = READ_WORD(left_body_muscle, i);
    uint16_t right_id = READ_WORD(right_body_muscle, i);

    int16_t left_val = ctm_get_weight(ctm, left_id);
    int16_t right_val = ctm_get_weight(ctm, right_id);

    if(left_val < 0) {
      left_val = 0;
    }

    if(right_val < 0) {
      right_val = 0;
    }

    body_total += (left_val + right_val);
  }

  uint16_t norm_body_total = 255.0 * ((float) body_total) / 600.0;

  // Gather total for neck muscles
  uint16_t left_neck_total = 0;
  uint16_t right_neck_total = 0;
  for(int i = 0; i < NECK_MUSCLES; i++) {
    uint16_t left_id = READ_WORD(left_neck_muscle, i);
    uint16_t right_id = READ_WORD(right_neck_muscle, i);

    int16_t left_val = ctm_get_weight(ctm, left_id);
    int16_t right_val = ctm_get_weight(ctm, right_id);

    if(left_val < 0) {
      left_val = 0;
    }

    if(right_val < 0) {
      right_val = 0;
    }

    left_neck_total += left_val;
    right_neck_total += right_val;
  }

  // Combine neck contribution with body
  int32_t neck_contribution = left_neck_total - right_neck_total;
  int32_t left_total;
  int32_t right_total;
  if(neck_contribution < 0) {
    left_total = 6*abs(neck_contribution) + norm_body_total;
    right_total = norm_body_total;
  }
  else {
    left_total = norm_body_total;
    right_total = 6*abs(neck_contribution) + norm_body_total;
  }

  // Log A and B type motor neuron activity
  double motor_neuron_sum = 0;

  /*
  for(int i = 0; i < MOTOR_B; i++) {
    uint16_t id = READ_WORD(motor_neuron_b, i);
    motor_neuron_sum += ctm_get_discharge(ctm, id);
  }
  */

  for(int i = 0; i < MOTOR_A; i++) {
    uint16_t id = READ_WORD(motor_neuron_a, i);
    motor_neuron_sum += ctm_get_discharge(ctm, id);
  }

  //const double motor_total = MOTOR_A + MOTOR_B;
  const double motor_total = MOTOR_A;
  const int avg_window = 15;
  double motor_neuron_percent = 100.0 * motor_neuron_sum / motor_total;

  _motorFireAvg = (motor_neuron_percent + (avg_window*_motorFireAvg))/(avg_window + 1.0);

  if(_motorFireAvg > 19.0) { // Magic number read off from c_matoduino simulation
    left_total *= -1;
    right_total *= -1;
  }

  _leftMuscle = left_total;
  _rightMuscle = right_total;
}
