# Bumba Meu Baja - Telemetria

<div align="center">
<img src="img/logoBmb.png" alt="Projeto Completo" width="30%"/>
<br>
</div>

## 📘 Descrição Geral
Este código foi desenvolvido para o **ESP32**, com o objetivo de realizar medições simultâneas de **RPM**, **temperatura** e **velocidade linear** (km/h), exibindo as informações em um **display Nextion**.  
Os sensores utilizados são:
- **Sensor Hall** para medição de RPM;
- **Sensor Hall** secundário para cálculo da velocidade;
- **Sensor de temperatura DS18B20** via barramento **OneWire**.

A comunicação entre o ESP32 e o display Nextion ocorre por meio da **UART2 (Serial2)**.

---

## Pinagem

| Função                   | Pino    | Motivo               |
| ------------------------ | ------- | -------------------- |
| DS18B20 (OneWire)        | GPIO 4  | Compatível e livre   |
| Sensor Hall (RPM)        | GPIO 26 | Boa para interrupção |
| Sensor Hall (Velocidade) | GPIO 27 | Boa para interrupção |
| Nextion TX               | GPIO 17 | UART2 TX             |
| Nextion RX               | GPIO 16 | UART2 RX             |

---

## ⚙️ Estrutura Geral do Código

### 1. Inclusão de Bibliotecas
```cpp
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
```
Essas bibliotecas permitem:
- O uso das funções base do Arduino (`Arduino.h`);
- A comunicação com sensores **OneWire** (`OneWire.h`);
- A leitura de temperatura do sensor **DS18B20** (`DallasTemperature.h`).

---

### 2. Configuração dos Componentes Nextion
```cpp
const String nextionRPM  = "t1";
const String nextionTemp = "t2";
const String nextionSpeed = "t3";
```
Cada variável representa o **nome do componente de texto** criado na tela do display Nextion (por exemplo: *t1*, *t2*, *t3*).  
Esses nomes são usados ao enviar comandos para atualizar os valores de RPM, temperatura e velocidade.

---

### 3. Configuração do Sensor de Temperatura DS18B20
```cpp
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
```
Define o pino **GPIO4** como barramento de comunicação **OneWire**, inicializando as instâncias necessárias para o sensor de temperatura.

---

### 4. Configuração dos Sensores Hall
```cpp
volatile unsigned long pulseCountRPM = 0;
volatile unsigned long pulseCountSpeed = 0;

const int sensorRPM = 26;
const int sensorSpeed = 27;
```
Foram definidos dois sensores Hall:
- `sensorRPM` (GPIO26): mede rotações para cálculo de RPM;
- `sensorSpeed` (GPIO27): mede rotações para cálculo de velocidade linear.

As variáveis `volatile` são utilizadas porque são modificadas dentro de **interrupções**.

---

### 5. Parâmetros da Roda e Cálculo de Velocidade
```cpp
const float wheelDiameter = 1.76; // Diâmetro da roda
const float wheelCircumference = wheelDiameter * 3.1416;
```
Essas variáveis representam o **diâmetro** e o **perímetro** da roda monitorada.  
O valor do perímetro é utilizado para converter rotações em distância percorrida.

---

### 6. Controle de Tempo
```cpp
unsigned long lastRPMUpdate = 0;
unsigned long lastTempUpdate = 0;
unsigned long lastSpeedUpdate = 0;
```
Essas variáveis armazenam o último instante de atualização de cada sensor, garantindo medições periódicas com base em intervalos definidos (em milissegundos).

---

### 7. Funções de Interrupção
```cpp
void pulseDetectedRPM() { pulseCountRPM++; }
void pulseDetectedSpeed() { pulseCountSpeed++; }
```
São **rotinas de interrupção** chamadas automaticamente a cada detecção de pulso no sensor Hall, incrementando o contador correspondente.

---

### 8. Envio de Comandos ao Nextion
```cpp
void sendNextionCommand(const String &cmd) {
  Serial2.print(cmd);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}
```
O Nextion utiliza três bytes `0xFF` para indicar o **fim de cada comando**.  
Essa função facilita o envio de comandos formatados como strings, seguidos dos bytes terminadores obrigatórios.

---

### 9. Cálculo do RPM
```cpp
float calculateRPM() {
  noInterrupts();
  unsigned long count = pulseCountRPM;
  pulseCountRPM = 0;
  interrupts();
  float rpm = (count * 60000.0) / (sampleTime * pulsesPerRevolution);
  sendNextionCommand(nextionRPM + ".txt="" + String(rpm, 0) + """);
  return rpm;
}
```
Essa função:
1. Desativa interrupções momentaneamente para ler o contador com segurança;
2. Calcula o **RPM** com base no número de pulsos capturados;
3. Envia o valor para o display Nextion.

---

### 10. Leitura de Temperatura
```cpp
void updateTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if (tempC == DEVICE_DISCONNECTED_C) {
    sendNextionCommand(nextionTemp + ".txt="--"");
  } else {
    sendNextionCommand(nextionTemp + ".txt="" + String(tempC, 1) + """);
  }
}
```
Solicita a leitura do sensor DS18B20 e atualiza o valor no display.  
Se o sensor estiver desconectado, exibe “--”.

---

### 11. Cálculo da Velocidade (km/h)
```cpp
float calculateSpeed() {
  noInterrupts();
  unsigned long count = pulseCountSpeed;
  pulseCountSpeed = 0;
  interrupts();
  float mps = (count * wheelCircumference) / (1000.0 / sampleTime);
  float kmh = mps * 3.6;
  sendNextionCommand(nextionSpeed + ".txt="" + String(kmh, 1) + """);
  return kmh;
}
```
Converte os pulsos do sensor Hall em **velocidade linear**.  
A distância percorrida por pulso é convertida em **m/s** e, em seguida, para **km/h**.

---

### 12. Setup
```cpp
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  sensors.begin();
  pinMode(sensorRPM, INPUT_PULLUP);
  pinMode(sensorSpeed, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorRPM), pulseDetectedRPM, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorSpeed), pulseDetectedSpeed, RISING);
}
```
- Inicializa as comunicações **Serial** e **Serial2** (para o Nextion);
- Ativa os sensores e suas interrupções;
- Configura os pinos como entrada com pull-up interno.

---

### 13. Loop Principal
```cpp
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastRPMUpdate >= sampleTime) calculateRPM();
  if (currentMillis - lastTempUpdate >= 1000) updateTemperature();
  if (currentMillis - lastSpeedUpdate >= 500) calculateSpeed();
}
```
O loop verifica continuamente o tempo decorrido para atualizar as medições em intervalos específicos, evitando sobrecarga de processamento.

---

## 🧠 Conclusão
O sistema faz uso eficiente das interrupções para leitura de sensores Hall e controle temporal não bloqueante (`millis()`), permitindo medições precisas e atualização periódica dos valores no display Nextion.

Esse modelo pode ser expandido facilmente para incluir novos sensores ou funções de calibração.

---
**Autor:** Iori Rodrigues  
**Plataforma:** ESP32 (PlatformIO)  
**Framework:** Arduino  
**Versão:** 1.0  




