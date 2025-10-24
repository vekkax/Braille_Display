#include <Arduino.h>

// ===========================================================
// Dual-UART PING sender for STM32 UART test
// Sends "ping" on Serial1 and Serial2 every 500 ms,
// waits for "pong" response, and prints result to Serial.
// ===========================================================

#define BAUDRATE 115200
#define TIMEOUT_MS 1000  // how long to wait for pong

const char *PING_MSG = "ping";
const char *EXPECT = "pong";

unsigned long lastPing = 0;

void setup() {
  Serial.begin(115200);      // USB serial for debug
  Serial1.begin(BAUDRATE);   // UART #1 → STM32 UART1
  Serial2.begin(BAUDRATE);   // UART #2 → STM32 UART2

  Serial.println(F("Arduino Mega UART ping tester ready."));
  Serial.println(F("-> Sending to STM32 on Serial1 and Serial2."));
  Serial.println(F("   Expecting pong response.\n"));
}

bool waitForPong(HardwareSerial &port) {
  unsigned long t0 = millis();
  char buffer[8];
  uint8_t idx = 0;

  while (millis() - t0 < TIMEOUT_MS) {
    if (port.available()) {
      char c = port.read();
      if (idx < sizeof(buffer) - 1)
        buffer[idx++] = c;

      buffer[idx] = '\0';
      if (strstr(buffer, EXPECT)) {
        return true;  // got pong
      }
    }
  }
  return false;  // timeout
}

void loop() {
  unsigned long now = millis();
  if (now - lastPing >= 500) {
    lastPing = now;

    Serial1.write((const uint8_t *)PING_MSG, 4);
    Serial2.write((const uint8_t *)PING_MSG, 4);

    Serial.print(F("Ping sent -> "));

    bool ok1 = waitForPong(Serial1);
    bool ok2 = waitForPong(Serial2);

    if (ok1) Serial.print(F("UART1 OK  "));
    else     Serial.print(F("UART1 TIMEOUT  "));

    if (ok2) Serial.print(F("UART2 OK"));
    else     Serial.print(F("UART2 TIMEOUT"));

    Serial.println();
  }
}