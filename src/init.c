#include "init.h"

void init_leds() {
    gpio_init(LED_BLUE_PIN);
    gpio_init(LED_RED_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);

}

void init_buttons() {
    gpio_init(B1_PIN);
    gpio_set_dir(B1_PIN, GPIO_IN);    
    gpio_pull_up(B1_PIN);
    
}


void init_i2c(ssd1306_t *ssd) {
    i2c_init(I2C_PORT, 400 * 1000); // Ensure correct I2C port is used
    gpio_set_function(LCD_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(LCD_SCL_PIN, GPIO_FUNC_I2C);   
    gpio_pull_up(LCD_SCL_PIN);
    gpio_pull_up(LCD_SDA_PIN); 

    ssd1306_init(ssd, WIDTH, HEIGHT, 0, ENDR_LCD, I2C_PORT);
    ssd1306_config(ssd);
    ssd1306_send_data(ssd);
}

uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);  
    return slice_num; 
}

void adc_start() {
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);
    adc_gpio_init(POT_PIN);
}