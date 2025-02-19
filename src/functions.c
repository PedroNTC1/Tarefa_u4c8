#include "functions.h"
#define JOYSTICK_SENSITIVITY 0.02  // lower value reduces sensitivity

void joystick_define_intensity(float *led_intensity, uint16_t ex_y) {
    if(*led_intensity < 0) *led_intensity = 0;
    if(*led_intensity > 4095) *led_intensity = 4095.0;
    if(*led_intensity >= 0) {
        if(ex_y > 2047 && *led_intensity < 4095) {
            float diff = ex_y - 2047;
            *led_intensity += diff * JOYSTICK_SENSITIVITY;  // smooth addition with sensitivity factor
            printf("LED Intensity: %f\n", *led_intensity);
        }
        if(ex_y < 2047 && *led_intensity > 0) {
            float diff = 2047 - ex_y;
            *led_intensity -= diff * JOYSTICK_SENSITIVITY;  // smooth subtraction with sensitivity factor
            printf("LED Intensity: %f\n", *led_intensity);
        }
    }

}

// Revert update_position to use calibrated center values and thresholds
void update_position(uint16_t x_value, uint16_t y_value, int* x_pos, int* y_pos) {
    // Calibrated center values (adjust if needed)
    const int center_x = 1800;
    const int center_y = 1918;
    // Increased thresholds to reduce ghost movement.
    const int threshold_x = 100;
    const int threshold_y = 80;

    int error_x = x_value - center_x;
    int error_y = y_value - center_y;

    printf("Error x: %d, Error y: %d\n", error_x, error_y);


    if(error_x < 260 && error_x > -240) {
        return;
    }

    if(error_x < -10 && error_x > 50) {
        return;
    }

    if (error_x > threshold_x && *x_pos < 50) {
        (*x_pos)++;
    } else if (error_x < -threshold_x && *x_pos > 0) {
        (*x_pos)--;
    }

    if (error_y > threshold_y && *y_pos < 120) {
        (*y_pos)++;
    } else if (error_y < -threshold_y && *y_pos > 0) {
        (*y_pos)--;
    }
}