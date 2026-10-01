```text
    _.-.-._
  .(   _   ).
 (  .-' '-.  )
(  (   O   )  )    ESTUFA INTELIGENTE
 (  '-._.-'  )     IoT Greenhouse Project
  '(_______)'
      | |
      | |
```
# Estufa Inteligente com ESP32

## Mostra Técnica ETEC Philadelpho 2026

Projeto desenvolvido para a Mostra Técnica da ETEC Philadelpho 2026, focado no monitoramento e análise em tempo real das condições de cultivo em estufas através da Internet das Coisas (IoT).

## Objetivo

O projeto consiste em um sistema de monitoramento de estufa inteligente utilizando um **ESP32**, um sensor de temperatura e umidade (**DHT11**) e um sensor de luminosidade (**LDR**).

O sistema hospeda seu próprio servidor web local (Access Point), permitindo que o usuário acesse um dashboard interativo pelo celular ou computador. Na interface web, o usuário seleciona a cultura agrícola desejada (ex.: Rúcula, Alface, Manjericão, Tomate, Morango) e o sistema compara as leituras dos sensores em tempo real com as faixas ideais para aquela planta, fornecendo diagnósticos, explicações botânicas e sugestões de ajustes.

## Funcionamento

O ESP32 opera criando uma rede Wi-Fi própria (`Estufa_ESP32`), servindo uma interface web moderna diretamente de sua memória.

A lógica de funcionamento é:

- Os sensores **DHT11** e **LDR** realizam leituras contínuas do ambiente (temperatura, umidade e luminosidade).
- O ESP32 processa os dados e pisca um **LED Verde indicador** a cada ciclo de leitura para sinalizar o correto funcionamento do circuito.
- O usuário conecta-se à rede Wi-Fi do ESP32 e acessa o site pelo navegador no endereço `http://192.168.4.1`.
- Na interface web, o usuário seleciona ou pesquisa a planta cultivada na estufa.
- O dashboard exibe os valores atuais, os intervalos ideais e barrinhas gráficas indicativas para cada parâmetro.
- A aplicação compara os dados reais com os limites e indica o estado da estufa: **Dentro do ideal**, **Quase no ideal** ou **Fora do ideal**.
- O site exibe explicações técnicas do motivo daqueles parâmetros para a planta escolhida e sugere correções operacionais (ex.: ventilação, irrigação, sombreamento).

## Tecnologias utilizadas

![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![HTML5](https://img.shields.io/badge/HTML5-E34F26?style=for-the-badge&logo=html5&logoColor=white)
![CSS3](https://img.shields.io/badge/CSS3-1572B6?style=for-the-badge&logo=css3&logoColor=white)
![JavaScript](https://img.shields.io/badge/JavaScript-F7DF1E?style=for-the-badge&logo=javascript&logoColor=black)
![Arduino IDE](https://img.shields.io/badge/Arduino_IDE-00979D?style=for-the-badge&logo=arduino&logoColor=white)

### Linguagens e Tecnologias Web

- C++ (Hardware e WebServer)
- HTML5 / CSS3 / JavaScript (Interface do usuário hospedada no ESP32)

### Microcontrolador

- ESP32 (Atuando como servidor web e processador IoT)

### Componentes de Hardware

- Placa ESP32
- Sensor de Temperatura e Umidade **DHT11**
- Sensor de Luminosidade **LDR** (com resistor de 10kΩ em divisor de tensão)
- LED Verde indicador de status (GPIO 2)
- Protoboard e jumpers para montagem do circuito

## Bibliotecas Utilizadas

O projeto utiliza bibliotecas nativas do ecossistema ESP32 e da Adafruit para leitura de sensores e criação do servidor de arquivos:

- `<WiFi.h>` e `<WebServer.h>`: Criação do ponto de acesso Wi-Fi e do servidor HTTP.
- `<DHT.h>`: Leitura dos dados de temperatura e umidade do sensor DHT11.

## Dashboard Web e Monitoramento

A interface web é embarcada no próprio código do ESP32 (`PROGMEM`) para evitar dependência de conexão externa com a internet ou servidores na nuvem.

Principais recursos da interface:
- **Catálogo de Plantas:** Banco de dados interno contendo informações nutricionais/ambientais de diversas culturas.
- **Requisições assíncronas (Fetch API):** Atualização automática dos dados na tela a cada 2 segundos via rota JSON (`/dados`).
- **Feedback Visual:** Cards que mudam de cor dinamicamente conforme a adequação do clima da estufa para a planta selecionada.
