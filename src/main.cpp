#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ------------------- Configurações Nextion -------------------
const String nextionRPM  = "t1"; // componente para RPM
const String nextionTemp = "t4"; // componente para temperatura

// ------------------- Configuração do DS18B20 -------------------
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ------------------- Configurações do sensor Hall -------------------
volatile unsigned long pulseCount = 0;
const int sensorPin = 2;
const unsigned int pulsesPerRevolution = 1;
const unsigned long sampleTime = 200; // intervalo de medição do RPM em ms

// ------------------- Controle de tempo -------------------
unsigned long lastRPMUpdate  = 0;
unsigned long lastTempUpdate = 0;

// ------------------- Função de interrupção -------------------
void pulseDetected() {
  pulseCount++; // incrementa pulsos do sensor
}

// ------------------- Função para enviar comandos ao Nextion -------------------
void sendNextionCommand(const String &cmd) {
  Serial1.print(cmd);
  Serial1.write(0xFF);
  Serial1.write(0xFF);
  Serial1.write(0xFF);
}

// ------------------- Função para calcular RPM -------------------
float calculateRPM() {
  noInterrupts();           // desativa interrupções para ler pulseCount com segurança
  unsigned long count = pulseCount;
  pulseCount = 0;           // zera contador
  interrupts();             // reativa interrupções

  float rpm = (count * 60000.0) / (sampleTime * pulsesPerRevolution); // cálculo RPM
  Serial.print("RPM: ");
  Serial.println(rpm);
  sendNextionCommand(nextionRPM + ".txt=\"" + String(rpm, 0) + "\"");
  return rpm;
}

// ------------------- Função para ler temperatura -------------------
void updateTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor DS18B20 não encontrado!");
    sendNextionCommand(nextionTemp + ".txt=\"--\"");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(tempC);
    Serial.println(" ºC");
    sendNextionCommand(nextionTemp + ".txt=\"" + String(tempC, 0) + "\"");
  }
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);

  // Inicializa sensor de temperatura
  sensors.begin();

  // Configura pino do sensor Hall com interrupção
  pinMode(sensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorPin), pulseDetected, RISING);

  Serial.println("Sistema inicializado.");
}

// ------------------- Loop -------------------
void loop() {
  unsigned long currentMillis = millis();

  // Atualiza RPM a cada sampleTime
  if (currentMillis - lastRPMUpdate >= sampleTime) {
    calculateRPM();
    lastRPMUpdate = currentMillis;
  }

  // Atualiza temperatura a cada 1000 ms
  if (currentMillis - lastTempUpdate >= 1000) {
    updateTemperature();
    lastTempUpdate = currentMillis;
  }
}
