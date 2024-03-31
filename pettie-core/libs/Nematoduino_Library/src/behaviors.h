#ifndef _BEHAVIORS_H
#define _BEHAVIORS_H

#define CHEMOTAXIS_LEN 8
#define NOSE_TOUCH_LEN 10

#include <stdint.h>
#include "utility/defines.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern const uint16_t NOSE_TOUCH[];
extern const uint16_t CHEMOTAXIS[];

#ifdef __cplusplus
}
#endif

#endif // _BEHAVIORS_H
