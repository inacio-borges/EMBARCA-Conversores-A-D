#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// definicao do joystick
#define JOY_X 27
#define JOY_Y 26
#define JOY_BUTTON 22

// definicao dos botoes
#define A_BUTTON 5

// definicao dos pinos para PWM
#define PWM_GREEN 11                       // para definir o pwm no led da BitDogLab
#define PWM_BLUE 12                        // para definir o pwm no led da BitDogLab
#define PWM_RED 13                         // para definir o pwm no led da BitDogLab
#define PWM_FREQ 50                        // Frequência do PWM em Hz
#define PWM_PERIOD_US (1000000 / PWM_FREQ) // Período do PWM em microssegundos (20ms)

const uint16_t WRAP_PERIOD = 20000; // valor máximo do contador - WRAP
const float PWM_DIVISER = 125.0f;   // divisor do clock para o PWM

// Função para definir a porcentagem do pwm
void set_pwm_percent(uint slice_num, uint channel, float percent)
{
    // O período de PWM para 50Hz é 20ms (20000us)
    float pulse_width_us = (percent / 100.0) * 20000;
    uint16_t level = (pulse_width_us * WRAP_PERIOD) / PWM_PERIOD_US;
    pwm_set_chan_level(slice_num, channel, level);
}

bool debounce()
{
    static uint32_t ultimo_tempo_botao = 0;
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    // 200ms de debounce
    if (tempo_atual - ultimo_tempo_botao < 200)
    {
        return false;
    }

    ultimo_tempo_botao = tempo_atual;
    return true;
}

bool flagJoy = true;
int flagJoyBorda = 0;
bool flagA = true;
void JOY_BUTTON_callback(uint gpio, uint32_t eventos)
{
    gpio_acknowledge_irq(gpio, eventos); // Garante que a interrupção foi reconhecida

    if (!debounce())
        return;

    printf("botao do joystick pressionado \n");

    flagJoy = !flagJoy;

    flagJoyBorda++;

    if (flagJoyBorda >= 5)
    {
        flagJoyBorda = 0;
    }
}

void A_BUTTON_callback(uint gpio, uint32_t eventos)
{
    gpio_acknowledge_irq(gpio, eventos); // Garante que a interrupção foi reconhecida

    if (!debounce())
        return;

    printf("botao A pressionado \n");

    flagA = !flagA;
    printf("flagA: %s\n", flagA ? "true" : "false");
}

void gpio_callback(uint gpio, uint32_t events)
{
    if (gpio == A_BUTTON)
    {
        A_BUTTON_callback(gpio, events);
    }
    else if (gpio == JOY_BUTTON)
    {
        JOY_BUTTON_callback(gpio, events);
    }
    else if (gpio == botaoB)
    {
        gpio_irq_handler(gpio, events);
    }
}

int main()
{

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);

    stdio_init_all();

    // Configuração do pull-up para o botão
    gpio_set_dir(A_BUTTON, GPIO_IN);
    gpio_pull_up(A_BUTTON);

    // cofiguração do joystick
    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);
    gpio_set_dir(JOY_BUTTON, GPIO_IN);
    gpio_pull_up(JOY_BUTTON);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    // Configuração da interrupção do botão (detecta borda de descida)
    gpio_set_irq_enabled_with_callback(A_BUTTON,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);
    gpio_set_irq_enabled_with_callback(JOY_BUTTON,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);

    gpio_set_irq_enabled_with_callback(botaoB,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);

    // configuração dos PWMs
    gpio_set_function(PWM_RED, GPIO_FUNC_PWM);   // habilitar o pino GPIO como PWM
    gpio_set_function(PWM_GREEN, GPIO_FUNC_PWM); // habilitar o pino GPIO como PWM
    gpio_set_function(PWM_BLUE, GPIO_FUNC_PWM);  // habilitar o pino GPIO como PWM

    uint slice_RED = pwm_gpio_to_slice_num(PWM_RED); // obter o canal PWM da GPIO
    pwm_set_clkdiv(slice_RED, PWM_DIVISER);          // define o divisor de clock do PWM
    pwm_set_wrap(slice_RED, WRAP_PERIOD);            // definir o valor de wrap

    uint slice_GREEN = pwm_gpio_to_slice_num(PWM_GREEN); // obter o canal PWM da GPIO
    pwm_set_clkdiv(slice_GREEN, PWM_DIVISER);            // define o divisor de clock do PWM
    pwm_set_wrap(slice_GREEN, WRAP_PERIOD);              // definir o valor de wrap

    uint slice_BLUE = pwm_gpio_to_slice_num(PWM_BLUE); // obter o canal PWM da GPIO
    pwm_set_clkdiv(slice_BLUE, PWM_DIVISER);           // define o divisor de clock do PWM
    pwm_set_wrap(slice_BLUE, WRAP_PERIOD);             // definir o valor de wrap

    uint channel_RED = pwm_gpio_to_channel(PWM_RED);
    uint channel_GREEN = pwm_gpio_to_channel(PWM_GREEN);
    uint channel_BLUE = pwm_gpio_to_channel(PWM_BLUE);

    pwm_set_enabled(slice_RED, true);   // habilita o pwm no slice correspondente
    pwm_set_enabled(slice_GREEN, true); // habilita o pwm no slice correspondente
    pwm_set_enabled(slice_BLUE, true);  // habilita o pwm no slice correspondente

    while (true)
    {
        // Limpa o display. O display inicia com todos os pixels apagados.
        ssd1306_fill(&ssd, false);
        ssd1306_send_data(&ssd);

        adc_select_input(1);
        uint16_t X_axis = adc_read();
        adc_select_input(0);
        uint16_t Y_axis = adc_read();

        int16_t Y_axis_ssd = 4096 - Y_axis; // espelha o eixo y para ficar mais intuitivo no display

        // calculo do valor dos eixos de -100 a 100
        double X_axis_percent = (X_axis - 2048) / 2048.0 * 100;
        double Y_axis_percent = (Y_axis - 2048) / 2048.0 * 100;
        double Y_axis_ssd_percent = (Y_axis_ssd - 2048) / 2048.0 * 28;
        double X_axis_ssd_percent = (X_axis - 2048) / 2048.0 * 56;
        double dead_zone = 8.0; // ajusta o ponto zero do joystik para ignorar a flutuaçao.

        // retira o sinal negativo dos valores

        // aplicar a dead zone
        X_axis_percent = (fabs(X_axis_percent) < dead_zone) ? 0 : X_axis_percent;
        Y_axis_percent = (fabs(Y_axis_percent) < dead_zone) ? 0 : Y_axis_percent;
        Y_axis_ssd_percent = (fabs(Y_axis_ssd_percent) < dead_zone) ? 0 : Y_axis_ssd_percent;
        X_axis_ssd_percent = (fabs(X_axis_ssd_percent) < dead_zone) ? 0 : X_axis_ssd_percent;

        ssd1306_rect(&ssd, Y_axis_ssd_percent + 28, X_axis_ssd_percent + 60, 8, 8, true, true); // Desenha um retângulo

        if (flagJoyBorda >= 1)
        {
            ssd1306_rect(&ssd, 0, 0, 128, 64, true, false); // Desenha uma borda
        }

        if (flagJoyBorda >= 2)
        {
            ssd1306_rect(&ssd, 2, 2, 124, 60, true, false); // Desenha duas bordas
        }

        if (flagJoyBorda >= 3)
        {
            ssd1306_rect(&ssd, 5, 5, 118, 54, true, false); // Desenha tres bordas
        }

        if (flagJoyBorda >= 4)
        {
            ssd1306_rect(&ssd, 6, 6, 116, 52, true, false); // Desenha quatro bordas
        }

        if (flagJoyBorda >= 5)
        {
            ssd1306_rect(&ssd, 8, 8, 114, 50, true, false); // Desenha cinco bordas
        }

        ssd1306_send_data(&ssd); // Atualiza o display

        X_axis_percent = fabs(X_axis_percent);
        Y_axis_percent = fabs(Y_axis_percent);

        // botão A ativa e desativa o controle dos LEDs pwm pelo joystick a cada acionamento
        if (flagA)
        {
            set_pwm_percent(slice_RED, channel_RED, X_axis_percent);
            set_pwm_percent(slice_BLUE, channel_BLUE, Y_axis_percent);
            printf("eixo y%%: %f, eixo Y: %f \n", Y_axis_ssd_percent, X_axis_ssd_percent);
            // printf("eixo x%%: %f, eixo X: %u \n", X_axis_percent, X_axis);

            // controle do led verde de acordo com o botao do joystick
            set_pwm_percent(slice_GREEN, channel_GREEN, flagJoy ? 0 : 100);
            sleep_ms(5);
        }

        sleep_ms(10);
    }
}
