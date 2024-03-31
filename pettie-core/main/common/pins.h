/*
 * ESP32's definitions for GPIOs
 * 
 */

#ifndef _PINS_H
#define _PINS_H


// ----------------------------------------------------------------------
#define GENTLE_TOUCH_TAIL_PIN (35) // IO35

// #define GENTLE_TOUCH_MID_PIN (34) // IO34

// ----------------------------------------------------------------------
#define SERVO_D5 (26) // back right low leg
#define SERVO_D6 (25) // back right upper leg
#define SERVO_D7 (33) // front right upper leg
#define SERVO_D8 (32) // front right low leg
#define SERVO_D0 (13) // back left low leg
#define SERVO_D1 (12) // back left upper leg
#define SERVO_D2 (14) // front left upper leg
#define SERVO_D4 (27) // front left low leg


// ----------------------------------------------------------------------
// For ESP32 only
#define I2C_SDA_PIN 21 // IO27
#define I2C_SCL_PIN 22 // IO26

// ----------------------------------------------------------------------
#define UART_2_TX_PIN 17 // IO17
#define UART_2_RX_PIN 16 // IO16




#endif // _PINS_H
