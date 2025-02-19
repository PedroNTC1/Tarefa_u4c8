#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>      // only include what is needed here
#include <stdint.h>

// Forward declaration of function
void joystick_define_intensity(float *led_intensity, uint16_t ex_y);
void update_position(uint16_t x_value, uint16_t y_value, int* x_pos, int* y_pos);

#endif // FUNCTIONS_H