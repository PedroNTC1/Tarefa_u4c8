#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "src/init.h" 
#include "src/functions.h"
#include <stdlib.h>  // for abs

ssd1306_t ssd;


int main()
{
    stdio_init_all();

    // Inicializa o LCD
    init_i2c(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa o ADC
    adc_start();

    pwm_init_gpio(LED_BLUE_PIN, 4096);
    pwm_init_gpio(LED_RED_PIN, 4096);  // added: initialize red LED PWM
    pwm_init_gpio(LED_GREEN_PIN, 4096);

    int ledToggle = 0; // persistent toggle state for LED_RED
    bool override = false;
    uint16_t last_joystick_x = 0;

    // New variables for LED_BLUE
    int ledToggleBlue = 0;
    bool overrideBlue = false;
    uint16_t last_joystick_y = 0;

    // Track if LED is controlled by joystick
    bool wasJoystickRed = true;
    bool wasJoystickBlue = true;

    // New variables for border & green LED toggle via joystick button (GPIO22)
    bool greenToggle = false;
    int borderIndex = 0;

    // Add new flag variable for manual override
    bool manualOverrideActive = false;

    // Add new variable for button-controlled LED state
    bool manualLEDOn = false;

    // NEW: Variables to track current LED states (0 means off)
    int currentRed = 0, currentBlue = 0;

    // Inicializa o botão A apenas uma vez
    gpio_init(B1_PIN);
    gpio_set_dir(B1_PIN, GPIO_IN);
    gpio_pull_up(B1_PIN);

    // Initialize POT_PIN (used for green LED toggle) to avoid floating state
    gpio_init(POT_PIN);
    gpio_set_dir(POT_PIN, GPIO_IN);
    gpio_pull_up(POT_PIN);

   uint32_t last_print_time = 0;

   float led_intensity_y = 0;
   float led_intensity_x = 0;

   int square_x = 0;
   int square_y = 0;
   
    while (1) {
        // Process joystick Y (for LED_BLUE)
        adc_select_input(0);
        uint16_t ex_y = adc_read();
        printf("Y: %d\n", ex_y);
        joystick_define_intensity(&led_intensity_y, ex_y);
                
        // Cancel blue override if joystick Y changed significantly
        if (abs((int)ex_y - (int)last_joystick_y) > 50) {
            overrideBlue = false;
        }
        last_joystick_y = ex_y;

        // Atualiza LED azul via joystick se não estiver em override
        if (!overrideBlue) {
            pwm_set_gpio_level(LED_BLUE_PIN, led_intensity_y);
            currentBlue = led_intensity_y;
            wasJoystickBlue = true;
        }

        // Atualiza LED azul via joystick se não estiver em override
        if (!overrideBlue) {
            pwm_set_gpio_level(LED_BLUE_PIN, led_intensity_y);
            wasJoystickBlue = true;
        }

        // Process joystick X (for LED_RED)
        adc_select_input(1);
        uint16_t ex_x = adc_read();
        printf("X: %d\n", ex_x);
        joystick_define_intensity(&led_intensity_x, ex_x);

        // Cancel override for red LED if joystick X changed significantly
        if (abs((int)ex_x - (int)last_joystick_x) > 50) {
            override = false;
        }
        last_joystick_x = ex_x;

        // Atualiza LED vermelho via joystick se não estiver em override
        if (!override) {
            pwm_set_gpio_level(LED_RED_PIN, led_intensity_x);
            currentRed = led_intensity_x;
            wasJoystickRed = true;
        }

        // Limpa o quadrado anterior
        ssd1306_square(&ssd, square_x, square_y, 8, false, true);

        // Move o quadrado
        update_position(ex_x, ex_y, &square_x, &square_y);

        // Renderiza e envia o quadrado
        ssd1306_square(&ssd, square_x, square_y, 8, true, true);
        ssd1306_send_data(&ssd);

        wasJoystickRed = true;
    

    // Limpa o quadrado anterior
    ssd1306_square(&ssd, square_x, square_y, 8, false, true);

    // Move o quadrado
    update_position(ex_x, ex_y, &square_x, &square_y);

    // chama a função para renderizar e enviar o quadrado
    ssd1306_square(&ssd, square_x, square_y, 8, true, true);
    ssd1306_send_data(&ssd);


       // 

        float duty_cycle = ((float)led_intensity_x / 4095)*100;

        uint32_t current_time = to_ms_since_boot(get_absolute_time());

        if (current_time - last_print_time > 1000)
        {
            printf("Duty Cycle: %.2f\n", duty_cycle);
            last_print_time = current_time;
           
        }

        // Use apenas o check:
        // Botão B1: botão sempre alterna o estado dos LEDs ignorando o valor do joystick
        if (gpio_get(B1_PIN) == 0) {
            sleep_ms(10);  // debounce inicial
            if (gpio_get(B1_PIN) == 0) {
                override = true;
                overrideBlue = true;
                if (currentRed == 0 && currentBlue == 0) {
                    // Se estar apagado, força ligar os LEDs com intensidade máxima
                    manualLEDOn = true;
                    currentRed = 4095; currentBlue = 4095;
                    pwm_set_gpio_level(LED_RED_PIN, 4095);
                    pwm_set_gpio_level(LED_BLUE_PIN, 4095);
                } else {
                    // Se estiver ligado (por joystick ou manual), força desligar
                    manualLEDOn = false;
                    currentRed = 0; currentBlue = 0;
                    pwm_set_gpio_level(LED_RED_PIN, 0);
                    pwm_set_gpio_level(LED_BLUE_PIN, 0);
                }
                sleep_ms(200); // debounce final
            }
        }

        // Nova verificação para o botão do JoyStick (GPIO22)
        if (gpio_get(POT_PIN) == 0) {
            sleep_ms(10);  // debounce inicial
            if (gpio_get(POT_PIN) == 0) {
                greenToggle = !greenToggle;
                borderIndex = (borderIndex + 1) % 3;
                pwm_set_gpio_level(LED_GREEN_PIN, greenToggle ? 4095 : 0);
                sleep_ms(200); // debounce final
            }
        }

        // Se o LED verde estiver ativo, desenha a borda conforme o estilo
        if (greenToggle) {
            switch(borderIndex) {
                case 0:
                    // Borda ocupando toda a tela
                    ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);
                    break;
                case 1:
                    // Borda com margem de 2 pixels
                    ssd1306_rect(&ssd, 2, 2, WIDTH - 4, HEIGHT - 4, true, false);
                    break;
                case 2:
                    // Borda com margem de 4 pixels
                    ssd1306_rect(&ssd, 4, 4, WIDTH - 8, HEIGHT - 8, true, false);
                    break;
                default:
                    break;
            }
            ssd1306_send_data(&ssd);
        }

        printf("end\n");
        sleep_ms(100);
    } 

    return 0;
}