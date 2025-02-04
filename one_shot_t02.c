#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Definição dos pinos
#define LED_VERMELHO 13
#define LED_VERDE 11
#define LED_AZUL 12
#define BUTTON_A 5

volatile bool leds_ativos = false; // indica se os LEDs estão em uma sequência ativa
absolute_time_t tempo_inicio; // armazena o tempo do início da sequência

// funçao de callback para desligar LEDs em sequência
int64_t turn_off_leds_callback(alarm_id_t id, void *user_data) {
    static int estado = 0;
    absolute_time_t tempo_atual = get_absolute_time();
    // converte para ms
    int64_t tempo_decorrido = absolute_time_diff_us(tempo_inicio, tempo_atual) / 1000;

    switch (estado) {
        case 0:
            gpio_put(LED_AZUL, 0);
            printf("[%lld ms] LED Azul apagado\n", tempo_decorrido);
            estado++;
            add_alarm_in_ms(3000, turn_off_leds_callback, NULL, true);
            break;

        case 1:
            gpio_put(LED_VERDE, 0);
            printf("[%lld ms] LED Verde apagado\n", tempo_decorrido);
            estado++;
            add_alarm_in_ms(3000, turn_off_leds_callback, NULL, true);
            break;

        case 2:
            gpio_put(LED_VERMELHO, 0);
            printf("[%lld ms] LED Vermelho apagado\n", tempo_decorrido);
            estado = 0;
            leds_ativos = false; // Permite reiniciar o processo ao pressionar o botão
            printf("Todos os LEDs foram apagados. Você pode pressionar o botão novamente.\n");
            break;
    }
    return 0;
}

// debounce para o botão
bool debounce_button() {
    if (gpio_get(BUTTON_A) == 0) {
        sleep_ms(50); // pequeno atraso para debounce
        if (gpio_get(BUTTON_A) == 0) {
            while (gpio_get(BUTTON_A) == 0) {
                // aguarda o botão ser solto
            }
            return true;
        }
    }
    return false;
}

// função para iniciar o processo quando o botão for pressionado
void check_button() {
    if (leds_ativos) {
        printf("[Aviso] O botão foi pressionado, mas os LEDs ainda estão no processo de desligamento. Aguarde.\n");
        return;
    }
    
    if (debounce_button()) {
        leds_ativos = true;
        tempo_inicio = get_absolute_time(); // Marca o tempo de início

        // Acende todos os LEDs
        gpio_put(LED_VERMELHO, 1);
        gpio_put(LED_VERDE, 1);
        gpio_put(LED_AZUL, 1);
        printf("[0 ms] Todos os LEDs acesos!\n");

        // Inicia a sequência de desligamento após 3 segundos
        add_alarm_in_ms(3000, turn_off_leds_callback, NULL, true);
    }
}

int main() {
    stdio_init_all();

    // Configuração dos pinos
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A); // Habilita pull-up interno no botão

    printf("Sistema pronto! Pressione o botão A para iniciar...\n");

    while (true) {
        check_button(); // Verifica se o botão foi pressionado
        sleep_ms(10); // Delay pequeno para otimização
    }
}