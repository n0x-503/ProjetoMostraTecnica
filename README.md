# Sistema de Automação com ESP32

## Mostra Técnica ETEC Philadelpho 2026

Projeto desenvolvido para a Mostra Técnica da ETEC Philadelpho 2026, com o objetivo de desenvolver um sistema de automação capaz de monitorar a temperatura e a umidade de um ambiente e realizar uma ação automática de acordo com os limites definidos pelo usuário.

## Objetivo

O projeto consiste em um sistema de automação utilizando um **ESP32** e sensores para realizar o monitoramento da **temperatura** e da **umidade** do ambiente.

O usuário pode determinar um limite máximo para a temperatura e outro para a umidade. Quando um dos valores medidos ultrapassa o limite estabelecido, o sistema aciona automaticamente uma lâmpada.

A temperatura atual do ambiente é apresentada em um **display de 7 segmentos utilizando o módulo TM1637**, permitindo que o usuário acompanhe as informações em tempo real.

## Funcionamento

O sistema realiza continuamente a leitura dos sensores de temperatura e umidade. Os valores obtidos são processados pelo ESP32 e comparados com os limites definidos pelo usuário.

A lógica de funcionamento é:

- O sensor realiza a leitura da temperatura e da umidade.
- O ESP32 recebe e processa os valores.
- O usuário define o limite de temperatura.
- O usuário define o limite de umidade.
- A temperatura atual é exibida no display TM1637.
- Caso a temperatura ultrapasse o limite definido, a lâmpada é acionada.
- Caso a umidade ultrapasse o limite definido, a lâmpada também é acionada.
- Quando os valores permanecem dentro dos limites estabelecidos, a lâmpada permanece desligada.

## Tecnologias utilizadas

![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Visual Studio Code](https://img.shields.io/badge/Visual%20Studio%20Code-007ACC?style=for-the-badge&logo=visualstudiocode&logoColor=white)
![PlatformIO](https://img.shields.io/badge/PlatformIO-F5822A?style=for-the-badge&logo=platformio&logoColor=white)
![Wokwi](https://img.shields.io/badge/Wokwi-000000?style=for-the-badge&logo=wokwi&logoColor=white)

### Linguagem

- C++

### Microcontrolador

- ESP32

### Ambiente de desenvolvimento

- Visual Studio Code
- PlatformIO

### Simulação

- Wokwi

### Componentes

- ESP32
- Sensor de temperatura e umidade
- Display de 7 segmentos
- Módulo TM1637
- Lâmpada
- Componentes necessários para a montagem do circuito

## Bibliotecas

O projeto utiliza bibliotecas para realizar a comunicação entre o ESP32, os sensores e o módulo de display TM1637.

As dependências do projeto são gerenciadas pelo **PlatformIO**.

## Display TM1637

O módulo **TM1637** é utilizado para controlar um display de 7 segmentos.

No projeto, ele é responsável por apresentar a temperatura atual medida pelo sensor, permitindo o acompanhamento das condições do ambiente.

## Automação

A automação é baseada na comparação entre os valores atuais medidos pelos sensores e os limites definidos pelo usuário.

A lâmpada será acionada quando pelo menos uma das condições for atendida:

```cpp
temperatura > temperaturaLimite
