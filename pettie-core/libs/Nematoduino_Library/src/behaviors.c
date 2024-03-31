#include <stdlib.h>

#include "behaviors.h"

// Arrays for neurons describing 'nose touch' and
// food-seeking behaviors
const uint16_t NOSE_TOUCH[] = {
  N_FLPR, N_FLPL, N_ASHL, N_ASHR, N_IL1VL, N_IL1VR,
  N_OLQDL, N_OLQDR, N_OLQVR, N_OLQVL
};

const uint16_t CHEMOTAXIS[] = {
  N_ADFL, N_ADFR, N_ASGR, N_ASGL, N_ASIL, N_ASIR,
  N_ASJR, N_ASJL
};


#if 1
// Gentle touch
// const uint16_t BODY_MID_TOUCH[] = {
//     // backward escape
//     N_AVM, N_ALMR, N_ALML,
//     // N_AWAL, N_AWAR
// };

// // Gentle touch
// const uint16_t BODY_TAIL_TOUCH[] = {
//     // forward escape
//     N_PVM, N_PLMR, N_PLML
//     // N_PVNL, N_PVNR
// };

// Gentle touch for the left side
const uint16_t BODY_LEFT_TOUCH[] = {
    N_PLML, N_PVDL, N_PDEL, N_PVM, N_LUAL
};

// Gentle touch for the right side
const uint16_t BODY_RIGHT_TOUCH[] = {
    N_PLMR, N_PVDR, N_PDER, N_PVM, N_LUAR
};


// const uint16_t NOSE_TOUCH[] = {
//   N_FLPR, N_FLPL, N_ASHL, N_ASHR,
//   N_OLQDL, N_OLQDR, N_OLQVR, N_OLQVL
// };

// const uint16_t CHEMOTAXIS[] = {
//   N_ADFL, N_ADFR, N_ASGR, N_ASGL, N_ASIL, N_ASIR,
//   N_ASJR, N_ASJL, AWCL, AWCR, AWAL, AWAR
// };

#endif