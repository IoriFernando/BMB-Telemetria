#include <Arduino.h>

void setup() {
  Serial.begin(115200);   // USB para debug
  Serial1.begin(9600);    // UART1 para Nextion (ajuste baud rate conforme a tela)

  Serial.println("Teste Nextion: escrevendo texto...");
  Serial1.print("t0.txt=\"Iori Fernando\""); 
  Serial1.write(0xFF);  // terminador 1
  Serial1.write(0xFF);  // terminador 2
  Serial1.write(0xFF);  // terminador 3
}

void loop() {

}
