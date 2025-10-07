#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ------------------- Configurações Nextion -------------------
const String nextionRPM  = "t1"; // componente para RPM
const String nextionTemp = "t2"; // componente para temperatura
const String nextionSpeed = "t3"; // componente para velocidade (km/h)

// ------------------- Configuração do DS18B20 -------------------
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ------------------- Configurações dos sensores Hall -------------------
volatile unsigned long pulseCountRPM = 0;
volatile unsigned long pulseCountSpeed = 0;

const int sensorRPM = 26;     // pino para RPM
const int sensorSpeed = 27;   // pino para velocidade
const unsigned int pulsesPerRevolution = 1;
const unsigned long sampleTime = 200; // ms

// ------------------- Parâmetros do sensor de velocidade -------------------
const float wheelDiameter = 1.76; // diâmetro da rota metros
const float wheelCircumference = wheelDiameter * 3.1416; // metros por rotação
const unsigned int pulsesPerRevolutionSpeed = 1;

// ------------------- Controle de tempo -------------------
unsigned long lastRPMUpdate  = 0;
unsigned long lastTempUpdate = 0;
unsigned long lastSpeedUpdate = 0;

// ------------------- Funções de interrupção -------------------
void pulseDetectedRPM() {
  pulseCountRPM++;
}

void pulseDetectedSpeed() {
  pulseCountSpeed++;
}

// ------------------- Envio ao Nextion -------------------
// Cada comando enviado ao display Nextion deve ser finalizado com três bytes 0xFF,
// que indicam o término da instrução conforme o protocolo serial do Nextion.
void sendNextionCommand(const String &cmd) {
  Serial2.print(cmd);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}

// ------------------- Cálculo do RPM -------------------
float calculateRPM() {
  noInterrupts();
  unsigned long count = pulseCountRPM;
  pulseCountRPM = 0;
  interrupts();

  float rpm = (count * 60000.0) / (sampleTime * pulsesPerRevolution);
  Serial.print("RPM: ");
  Serial.println(rpm);
  sendNextionCommand(nextionRPM + ".txt=\"" + String(rpm, 0) + "\"");
  return rpm;
}

// ------------------- Leitura de temperatura -------------------
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

// ------------------- Cálculo da velocidade (km/h) -------------------
float calculateSpeed() {
  noInterrupts();
  unsigned long count = pulseCountSpeed;
  pulseCountSpeed = 0;
  interrupts();

  // Rotações por segundo (RPS)
  float rps = (count * 1000.0) / (1000.0 * pulsesPerRevolutionSpeed); 
  // Distância percorrida em metros por segundo
  float mps = (count * wheelCircumference) / (1000.0 / sampleTime);
  // Convertendo m/s para km/h
  float kmh = mps * 3.6;

  Serial.print("Velocidade: ");
  Serial.print(kmh);
  Serial.println(" km/h");

  sendNextionCommand(nextionSpeed + ".txt=\"" + String(kmh, 1) + "\"");

  return kmh;
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // UART2 para Nextion

  sensors.begin();

  // Configura pinos dos sensores Hall
  pinMode(sensorRPM, INPUT_PULLUP);
  pinMode(sensorSpeed, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorRPM), pulseDetectedRPM, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorSpeed), pulseDetectedSpeed, RISING);

  Serial.println("Sistema inicializado (ESP32).");
}

// ------------------- Loop -------------------
void loop() {
  unsigned long currentMillis = millis();

  // Atualiza RPM
  if (currentMillis - lastRPMUpdate >= sampleTime) {
    calculateRPM();
    lastRPMUpdate = currentMillis;
  }

  // Atualiza temperatura
  if (currentMillis - lastTempUpdate >= 1000) {
    updateTemperature();
    lastTempUpdate = currentMillis;
  }

  // Atualiza velocidade a cada 500 ms
  if (currentMillis - lastSpeedUpdate >= 500) {
    calculateSpeed();
    lastSpeedUpdate = currentMillis;
  }
}
