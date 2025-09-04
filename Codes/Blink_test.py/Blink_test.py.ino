// Arduino Mega – Comunicación con STM32 por Serial2 (pins 16 RX2, 17 TX2)

String palabra = "ROSA";         // Palabra objetivo
String desordenada = "OSAR";     // Letras para asignar (debe tener misma longitud)
int delayEntreMensajes = 300;

void setup() {
  Serial.begin(9600);         // Consola para el PC
  Serial2.begin(115200);      // UART2 -> STM32
  
  delay(1000);
  Serial.println("Iniciando secuencia de envío...");

  // Enviar letras por CHAR:index:letras_restantes
  String comando = "CHAR:0:" + desordenada;
  Serial2.println(comando);
  Serial.print("Enviado -> "); Serial.println(comando);
  delay(delayEntreMensajes);

  // Enviar palabra original
  comando = "WORD:" + palabra;
  Serial2.println(comando);
  Serial.print("Enviado -> "); Serial.println(comando);
  delay(delayEntreMensajes);

  // Enviar FIRST a la placa que tenga la primera letra de palabra
  Serial2.println("FIRST");
  Serial.println("Enviado -> FIRST");
  delay(delayEntreMensajes);

  // Enviar ANCHOR a la placa que tenga la última letra de palabra
  Serial2.println("ANCHOR");
  Serial.println("Enviado -> ANCHOR");
  delay(delayEntreMensajes);

  // Iniciar juego
  Serial2.println("START");
  Serial.println("Enviado -> START");
}

void loop() {
  // Reenvía lo que diga la STM32 al monitor serial
  if (Serial2.available()) {
    char c = Serial2.read();
    Serial.write(c);
  }
}
