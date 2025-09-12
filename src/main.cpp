#include <Arduino.h>           // Base do Arduino
#include <OneWire.h>           // Comunicação 1-Wire
#include <DallasTemperature.h> // Biblioteca do DS18B20

/*
  Projeto desenvolvido não estar utilizando a biblioteca do nextion
  por conta de problema de compatibilidade e por usuadilidade por isso 
  foi utilizado a comunicação serial para envio das informações.
  
  Hardware utilizado:
  - Arduino Mega
  - Nextion conectada na Serial1 (TX1 - pino 18, RX1 - pino 19)
  - Sensor DS18B20 no pino 4
*/

// ------------------- Configuração do DS18B20 -------------------
#define ONE_WIRE_BUS 4  // GPIO 4 (pino de dados do DS18B20)

// Instancia barramento OneWire
OneWire oneWire(ONE_WIRE_BUS);

// Passa o OneWire para a biblioteca do sensor
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);   // USB para debug
  Serial1.begin(9600);    // UART1 para Nextion (ajuste baud rate conforme a tela)

  Serial.println("Teste Nextion: escrevendo texto...");
  Serial1.print("t0.txt=\"Iori Fernando\""); 
  Serial1.write(0xFF);  // terminador 1
  Serial1.write(0xFF);  // terminador 2
  Serial1.write(0xFF);  // terminador 3


  sensors.begin();
}

void loop() {
  
  // Solicita leitura dos sensores DS18B20
  sensors.requestTemperatures(); 

  float tempC = sensors.getTempCByIndex(0); 

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor DS18B20 não encontrado!");
  }else {
    // Mostra no monitor serial (debug)
    Serial.print("Temperatura: ");
    Serial.print(tempC);
    Serial.println(" ºC");

    // ----------- Envia para o Nextion -----------
    Serial1.print("t1.txt=\"");   // abre comando
    Serial1.print(tempC);         // valor da temperatura
    Serial1.print(" C\"");        // fecha string (vai aparecer: 25.3 C)
    Serial1.write(0xFF);          // terminador 1
    Serial1.write(0xFF);          // terminador 2
    Serial1.write(0xFF);          // terminador 3
  }
}
