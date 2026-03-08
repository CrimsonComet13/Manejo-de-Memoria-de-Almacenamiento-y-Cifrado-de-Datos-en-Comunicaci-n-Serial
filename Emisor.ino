#include <SoftwareSerial.h>

SoftwareSerial linkSerial(10, 11); // RX, TX
const int stackSize = 4;
byte dataStack[stackSize];
int stackIndex = 0;
const byte encryptionKey = 0xAC;

unsigned long lastHeartbeat = 0;

void setup() {
  Serial.begin(9600);
  linkSerial.begin(4800); // Bajamos la velocidad para mayor estabilidad
  for (int i = 4; i <= 7; i++) pinMode(i, OUTPUT);
  Serial.println(">>> EMISOR INICIADO <<<");
}

void loop() {
  // 1. Envío de prueba automático cada 5 segundos para verificar conexión
  if (millis() - lastHeartbeat > 5000) {
    Serial.println("Enviando latido de prueba...");
    linkSerial.write(0xFF); // Un byte de prueba que el receptor reconocerá
    lastHeartbeat = millis();
  }

  // 2. Lógica de la antena (A0)
  int val = analogRead(A0);
  if (val > 60) { // Umbral de sensibilidad
    byte rawData = (byte)random(0, 255);
    if (stackIndex < stackSize) {
      dataStack[stackIndex] = rawData;
      digitalWrite(stackIndex + 4, HIGH);
      Serial.print("Pila ["); Serial.print(stackIndex); Serial.print("]: "); Serial.println(rawData, HEX);
      stackIndex++;
      delay(300); // Antirebote para el ruido de la antena
    }
  }

  // 3. Enviar cuando la pila esté llena
  if (stackIndex == stackSize) {
    enviarPaqueteCifrado();
    stackIndex = 0;
    for (int i = 4; i <= 7; i++) digitalWrite(i, LOW);
  }
}

void enviarPaqueteCifrado() {
  Serial.println("Cifrando y enviando...");
  linkSerial.write(0x7E); // Inicio '~'
  for (int i = 0; i < stackSize; i++) {
    byte encrypted = dataStack[i] ^ encryptionKey;
    linkSerial.write(encrypted);
    delay(80); // Tiempo para que el buffer del receptor no se sature
  }

  delay(80);
  linkSerial.write(0x7C); // Fin '|'
  Serial.println("¡Enviado!");
}
