# Projeto de controle de LEDs RGB com Joystick

Aluno: Inacio Oliveira Borges

link para o video da execucao do projeto: [Projeto em execucao](https://youtu.be/gRqNTdKJCIw)


## Descrição do Projeto

Este projeto utiliza um joystick para fornecer valores analógicos correspondentes aos eixos X e Y, que são utilizados para controlar a intensidade luminosa dos LEDs RGB. Além disso, um display SSD1306 é utilizado para exibir um quadrado de 8x8 pixels que se move proporcionalmente aos valores capturados pelo joystick.

### Funcionalidades Principais

- **Controle de LEDs RGB**:
  - **LED Azul**: Ajusta o brilho conforme o valor do eixo Y do joystick.
  - **LED Vermelho**: Ajusta o brilho conforme o valor do eixo X do joystick.
  - **LED Verde**: Alterna o estado a cada acionamento do botão do joystick.
- **Display SSD1306**:
  - Exibe um quadrado de 8x8 pixels, que se move conforme os valores dos eixos X e Y do joystick.
  - A borda do display é modificada a cada acionamento do botão do joystick.
- **Botão A**:
  - Ativa ou desativa os LEDs PWM a cada acionamento.

## Componentes Utilizados

- **LED RGB**: Conectado às GPIOs 11, 12 e 13.
- **Botão do Joystick**: Conectado à GPIO 22.
- **Joystick**: Conectado às GPIOs 26 e 27.
- **Botão A**: Conectado à GPIO 5.
- **Display SSD1306**: Conectado via I2C (GPIO 14 e GPIO 15).

## Requisitos do Projeto

1. **Uso de Interrupções**: Todas as funcionalidades relacionadas aos botões são implementadas utilizando rotinas de interrupção (IRQ).
2. **Debouncing**: Tratamento do bouncing dos botões via software.
3. **Utilização do Display 128x64**: Demonstrar o entendimento do princípio de funcionamento do display e a utilização do protocolo I2C.
4. **Organização do Código**: Manter uma boa organização e clareza no código.
