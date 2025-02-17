#include <stdio.h>             
#include "pico/stdlib.h"      
#include "hardware/adc.h"      
#include "hardware/pwm.h"  
#include "hardware/gpio.h"
#include "hardware/i2c.h"    

#define VRX_PIN 26 
#define VRY_PIN 27   
#define SW_PIN 22     
#define LED_BLUE_PIN 12  
#define LED_GREEN_PIN 11
#define LED_RED_PIN 13 
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6   

// Parâmetros para debounce
#define DEBOUNCE_DELAY_MS 50

// Variáveis para armazenar o estado dos botões
bool button_a_state = false;
bool button_b_state = false;

// Função para inicializar o PWM
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, false);
    return slice_num;
}

// Função para verificar o debounce de um botão
bool debounce(uint pin, bool *last_state) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    bool current_state = gpio_get(pin) == 0; // 0 indica que o botão está pressionado (ativo em nível baixo)

    if (current_state != *last_state) {
        if (current_time - last_time > DEBOUNCE_DELAY_MS) {
            last_time = current_time;
            *last_state = current_state;
            return current_state;
        }
    }
    return *last_state;
}

int main() {
    // Inicializa a comunicação serial para permitir o uso de printf
    stdio_init_all();

    // Inicializa o módulo ADC do Raspberry Pi Pico
    adc_init();
    adc_gpio_init(VRX_PIN); // Configura GP26 (ADC0) para o eixo X do joystick
    adc_gpio_init(VRY_PIN); // Configura GP27 (ADC1) para o eixo Y do joystick

    // Configura o pino do botão do joystick como entrada digital com pull-up interno
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN); // Habilita o pull-up interno para garantir leitura estável

    // Configura os botões A e B como entrada digital com pull-up interno
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN); // Habilita o pull-up interno para garantir leitura estável

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN); // Habilita o pull-up interno para garantir leitura estável

    // Inicializa os pinos dos LEDs como saída e desliga-os inicialmente
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, false);

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, false);

    // Inicializa o PWM para os LEDs azul e verde
    uint pwm_wrap = 4096;
    uint blue_pwm_slice = pwm_init_gpio(LED_BLUE_PIN, pwm_wrap);
    uint green_pwm_slice = pwm_init_gpio(LED_GREEN_PIN, pwm_wrap);

    uint32_t last_print_time = 0; // Variável para controlar o tempo de impressão na serial

    while (true) {
        // Leitura do valor do ADC para VRX (Eixo X do joystick)
        adc_select_input(0); // Seleciona canal 0 (GP26 - VRX)
        uint16_t vrx_value = adc_read(); // Lê o valor do eixo X, de 0 a 4095

        // Leitura do valor do ADC para VRY (Eixo Y do joystick)
        adc_select_input(1); // Seleciona canal 1 (GP27 - VRY)
        uint16_t vry_value = adc_read(); // Lê o valor do eixo Y, de 0 a 4095

        // Leitura do estado do botão do joystick (SW)
        bool sw_value = gpio_get(SW_PIN) == 0; // 0 indica que o botão está pressionado

        // Verifica o estado dos botões com debounce
        bool button_a_pressed = debounce(BUTTON_A_PIN, &button_a_state); // Verifica se o botão A foi pressionado com debounce
        bool button_b_pressed = debounce(BUTTON_B_PIN, &button_b_state); // Verifica se o botão B foi pressionado com debounce

        // Controle do LED azul com PWM baseado no valor do ADC0 (VRX) se o botão A estiver pressionado
        if (button_a_pressed) {
            pwm_set_gpio_level(LED_BLUE_PIN, vrx_value); // Ajusta o duty cycle do LED azul
            pwm_set_enabled(blue_pwm_slice, true); // Ativa o PWM do LED azul
        }

        // Controle do LED verde com PWM baseado no valor do ADC1 (VRY) se o botão B estiver pressionado
        if (button_b_pressed) {
            pwm_set_gpio_level(LED_GREEN_PIN, vry_value); // Ajusta o duty cycle do LED verde
            pwm_set_enabled(green_pwm_slice, true); // Ativa o PWM do LED verde
        }

        // Se nenhum botão estiver pressionado, desativa os PWMs
        if (!button_a_pressed) {
            pwm_set_gpio_level(LED_BLUE_PIN, 0); // Define o duty cycle como 0
            pwm_set_enabled(blue_pwm_slice, false); // Desativa o PWM do LED azul
        }
        
        if (!button_b_pressed) {
            pwm_set_gpio_level(LED_GREEN_PIN, 0); // Define o duty cycle como 0
            pwm_set_enabled(green_pwm_slice, false); // Desativa o PWM do LED verde
        }

        // Calcula o duty cycle em porcentagem para impressão
        float blue_duty_cycle = (vrx_value / 4095.0) * 100;
        float green_duty_cycle = (vry_value / 4095.0) * 100;

        // Imprime os valores lidos e o duty cycle proporcional na comunicação serial a cada 1 segundo
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_print_time >= 1000) {
            printf("VRX: %u, VRY: %u, SW: %d\n", vrx_value, vry_value, sw_value);
            printf("Button A: %s, Button B: %s\n", button_a_pressed ? "Pressed" : "Released", button_b_pressed ? "Pressed" : "Released");
            printf("Duty Cycle LED Azul: %.2f%%, Duty Cycle LED Verde: %.2f%%\n", blue_duty_cycle, green_duty_cycle);
            last_print_time = current_time; // Atualiza o tempo da última impressão
        }

        // Introduz um atraso de 100 milissegundos antes de repetir a leitura
        sleep_ms(100);
    }

    return 0;
}
