unsigned long ultimoEnvio = 0;

void setup() {
  Serial.begin(9600);     // USB para ver en el monitor serial
  Serial1.begin(9600);    // UART1 (TX1: pin 18, RX1: pin 19)
}

void loop() {
  // Enviar "HOLA" cada 5 segundos sin bloquear
  if (millis() - ultimoEnvio >= 5000) {
    Serial1.println("HOLA PICO");
    Serial.println("Enviado: HOLA PICO");
    ultimoEnvio = millis();
  }

  // Leer si hay algo por UART1
  if (Serial1.available()) {
    String recibido = Serial1.readStringUntil('\n');
    Serial.print("Recibido por UART1: ");
    Serial.println(recibido);
  }
}