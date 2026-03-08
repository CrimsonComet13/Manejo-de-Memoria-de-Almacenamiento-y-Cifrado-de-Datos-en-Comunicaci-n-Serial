#include <SoftwareSerial.h>

SoftwareSerial linkSerial(10, 11); // RX, TX

const byte START_BYTE = 0x7E;
const byte END_BYTE = 0x7C;
const byte HEARTBEAT = 0xFF;

const int stackSize = 4;
const byte encryptionKey = 0xAC;

byte buffer[stackSize];
int index = 0;

enum State {
  WAIT_START,
  READ_DATA,
  WAIT_END
};

State currentState = WAIT_START;

void setup() {

  Serial.begin(9600);
  linkSerial.begin(4800);

  Serial.println(">>> RECEPTOR AVANZADO LISTO <<<");
}

void loop() {

  while (linkSerial.available()) {

    byte incoming = linkSerial.read();

    // -------- LATIDO --------
    if (incoming == HEARTBEAT) {
      Serial.println("[OK] Latido recibido");
      return;
    }

    switch (currentState) {

      // ----------------------
      // ESPERAR INICIO
      // ----------------------
      case WAIT_START:

        if (incoming == START_BYTE) {
          index = 0;
          currentState = READ_DATA;
          Serial.println("Inicio de paquete detectado");
        }

      break;

      // ----------------------
      // LEER DATOS
      // ----------------------
      case READ_DATA:

        buffer[index] = incoming;
        index++;

        if (index >= stackSize) {
          currentState = WAIT_END;
        }

      break;

      // ----------------------
      // ESPERAR FIN
      // ----------------------
      case WAIT_END:

        if (incoming == END_BYTE) {

          Serial.println("--- DATOS DESCIFRADOS ---");

          for (int i = 0; i < stackSize; i++) {

            byte descifrado = buffer[i] ^ encryptionKey;

            Serial.print("Dato ");
            Serial.print(i);

            Serial.print(" | Cifrado: 0x");
            Serial.print(buffer[i], HEX);

            Serial.print(" -> Descifrado: ");
            Serial.println(descifrado);
          }

          Serial.println("-------------------------");

        } else {

          Serial.println("[ERROR] Fin de paquete incorrecto");

        }

        currentState = WAIT_START;

      break;

    }
  }
}
