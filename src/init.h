#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "hardware/adc.h" 
#include "hardware/pwm.h" 

#define VRX_PIN 26
#define VRY_PIN 27
#define POT_PIN 22

void adc_start();
uint pwm_init_gpio(uint gpio, uint wrap);

// LEDS PINS
#define LED_BLUE_PIN 12
#define LED_RED_PIN 13
#define LED_GREEN_PIN 11

// BUTTONS PINS
#define B1_PIN 5

// I2C Pins TELA LCD
#define LCD_SDA_PIN 14
#define LCD_SCL_PIN 15
#define ENDR_LCD 0x3C
#define I2C_PORT i2c1


extern void init_leds();
extern void init_buttons();
extern void init_i2c(ssd1306_t *ssd);
extern uint pwm_init_gpio(uint gpio, uint wrap);
extern void adc_start();