#ifndef _WORM_H
#define _WORM_H

#include "utility/connectome.h"
#include "utility/muscles.h"

void init_worm(void);

void worm_chemotaxis();
void worm_noseTouch();

int worm_getLeftMuscle();
int worm_getRightMuscle();

Connectome *worm_getConnectome();

#endif // _WORM_H
