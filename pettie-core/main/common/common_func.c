#include "common.h"
#include "esp_timer.h"

#include <assert.h>

int32_t __attribute__((optimize("-O2")))
ul_map_val(const int32_t x, int32_t imin, int32_t imax, int32_t omin, int32_t omax)
{
    return (x - imin) * (omax - omin) / (imax - imin) + omin;
}

// void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax)
// {
//   // calculate duty, 8191 from 2 ^ 13 - 1
//   uint32_t duty = (8191 / valueMax) * min(value, valueMax);

//   // write duty to LEDC
//   ledcWrite(channel, duty);
// }


int64_t system_get_us_time(void)
{
    return esp_timer_get_time();
}