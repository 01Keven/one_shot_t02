### Sequência de LEDs com Controle por Botão A

#### Visão Geral
Este código implementa um sistema no Raspberry Pi Pico W, onde três LEDs (Vermelho, Verde e Azul) acendem simultaneamente quando o botão (BUTTON_A) é pressionado. Após 3 segundos, os LEDs começam a apagar em sequência, e o processo é controlado por um temporizador e por um mecanismo de debounce para o botão, evitando múltiplas leituras indesejadas.

![2025-02-04-11-08-19](https://github.com/user-attachments/assets/b99a5135-1a26-4596-adb2-4038f7cf7c01)

# Experimento com BitDogLab
- Os leds não aparecem completamente com as cores correspondentes ao do Wokwi pois é necessario uso de biblioteca para manipulação individual das cores dos LEDs.

https://github.com/user-attachments/assets/96b6a731-88f5-4177-b1d4-4777ebdeef7e

### Estrutura do Projeto

O projeto está organizado da seguinte forma:
```
one_shot_t02/
├── one_shot_t02.c           # Código fonte principal
├── CMakeLists.txt           # Configuração para compilação
├── pico_sdk_import.cmake    # Importação do SDK do Pico
├── wokwi.toml               # Configuração do Wokwi para simulação online
├── diagram.json             # Arquivo de diagrama da fiação

```
---

### Como Clonar o Projeto

Para clonar este repositório, execute:
```
git clone https://github.com/seu-usuario/one_shot_t02.git
```

### Estrutura do Código

1. **Bibliotecas Importadas**
   
   ```c
   #include <stdio.h>
   #include "pico/stdlib.h"
   #include "hardware/gpio.h"
   #include "hardware/timer.h"
   ```

   - **`stdio.h`**: Biblioteca padrão de entrada e saída para o uso das funções de impressão (`printf`).
   - **`pico/stdlib.h`**: Contém funções úteis para o gerenciamento de entradas e saídas no Raspberry Pi Pico, como `gpio_init`, `gpio_set_dir`, e `sleep_ms`.
   - **`hardware/gpio.h`**: Contém funções para controlar os pinos GPIO do Raspberry Pi Pico.
   - **`hardware/timer.h`**: Fornece funções para trabalhar com temporizadores e alarmes, usados para controlar a sequência dos LEDs.

2. **Definição dos Pinos**

   ```c
   #define LED_VERMELHO 13
   #define LED_VERDE 12
   #define LED_AZUL 11
   #define BUTTON_A 5
   ```

   - **`LED_VERMELHO`, `LED_VERDE`, `LED_AZUL`**: Definem os pinos GPIO 13, 12 e 11 para os LEDs vermelho, verde e azul, respectivamente.
   - **`BUTTON_A`**: Define o pino GPIO 5 para o botão de controle.

3. **Declaração de Variáveis**

   ```c
   volatile bool leds_ativos = false; // indica se os LEDs estão em uma sequência ativa
   absolute_time_t tempo_inicio; // armazena o tempo do início da sequência
   ```

   - **`leds_ativos`**: Variável global usada para indicar se a sequência de LEDs está em andamento. O modificador `volatile` é usado para evitar otimizações do compilador.
   - **`tempo_inicio`**: Armazena o tempo exato de início da sequência, usado para calcular o tempo decorrido durante a sequência de desligamento dos LEDs.

4. **Função de Callback do Temporizador**

   ```c
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
   ```

   - **`turn_off_leds_callback`**: Esta função é chamada a cada 3 segundos por um temporizador repetitivo. Ela apaga os LEDs em sequência:
     - **Caso 0**: Apaga o LED Azul, imprime o tempo decorrido e define um próximo alarme para apagar o LED Verde.
     - **Caso 1**: Apaga o LED Verde, imprime o tempo decorrido e define um próximo alarme para apagar o LED Vermelho.
     - **Caso 2**: Apaga o LED Vermelho, finaliza a sequência e redefine a variável `leds_ativos` para `false`, permitindo que o processo seja reiniciado ao pressionar o botão.

   - **`absolute_time_diff_us`**: A função calcula o tempo decorrido em microssegundos entre `tempo_inicio` e o tempo atual, e é convertida para milissegundos.
   
   - **`add_alarm_in_ms`**: Configura o temporizador para disparar novamente após 3000 milissegundos (3 segundos).

5. **Função de Debounce para o Botão**

   ```c
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
   ```

   - **`debounce_button`**: Função que implementa o controle de debounce para o botão. Quando o botão é pressionado, a função aguarda um pequeno intervalo (50 ms) e depois verifica novamente se o botão está pressionado. Se o botão for pressionado, a função retorna `true`; caso contrário, retorna `false`. O código aguarda até que o botão seja solto antes de continuar.

6. **Função para Iniciar o Processo ao Pressionar o Botão**

   ```c
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
   ```

   - **`check_button`**: Função que verifica se o botão foi pressionado. Se os LEDs já estão em processo de desligamento, a função imprime uma mensagem de aviso e retorna. Caso contrário, a função chama `debounce_button` para garantir que a leitura do botão seja precisa e, se pressionado, inicia o processo de acender os LEDs e configura o temporizador para desligá-los em sequência.

7. **Função Principal**

   ```c
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
   ```

   - **`main`**: A função principal do programa. Inicializa os pinos GPIO para os LEDs e o botão, e configura o botão com um resistor pull-up interno para garantir que o sinal lido seja estável. O sistema entra em um loop infinito, onde a função `check_button` é chamada para verificar se o botão foi pressionado. O sistema fica aguardando a interação do usuário.

---

### Explicação do Fluxo de Execução

1. **Inicialização**:
   - O sistema é inicializado com a configuração dos pinos dos LEDs e do botão. O botão é configurado com um resistor pull-up interno, garantindo que ele leia um valor lógico "alto" quando não pressionado.
   
2. **Ação ao Pressionar o Botão**:
   - Quando o botão é pressionado, os LEDs são acesos simultaneamente. O temporizador é então configurado para chamar a função de desligamento dos LEDs após 3 segundos.

3. **Desligamento Sequencial dos LEDs**:
   - A função `turn_off_leds_callback` é chamada a cada 3 segundos. Ela apaga os LEDs em sequência (azul, verde, vermelho) e imprime a mensagem de tempo decorrido.
   - Após apagar todos os LEDs, a variável `leds_ativos` é resetada para `false`, permitindo que o processo seja reiniciado ao pressionar o botão novamente.

4. **Controle de Debounce**:
   - O debounce garante que o botão seja lido corretamente e que múltiplos eventos de pressionamento não causem problemas no funcionamento do sistema.

---

### Especificações Aplicadas

- **Debounce do Botão**: O mecanismo de debounce implementado ajuda a evitar leituras erradas devido a ruídos elétricos ou múltiplos acionamentos rápidos do botão.
- **Controle de Sequência**: O uso de um temporizador para acionar a sequência de desligamento dos LEDs de forma controlada e escalonada.
- **Uso de Alarmes**: O sistema utiliza alarmes para controlar o tempo e disparar a sequência de desligamento dos LEDs, o que permite um fluxo controlado e eficiente.
