// Arduino Mega prueba básica STM32
// Envía la palabra ROJO desordenada como OOJR
// TX2 (17) → RX STM32
// RX2 (16) ← TX STM32
// GND común

#define STM32_left Serial2
#define STM32_right Serial3
#define PC    Serial
int del =200;

void setup() {
  PC.begin(115200);     // Consola en PC

  STM32_left.begin(115200);  // UART hacia STM32
  STM32_right.begin(115200);
  delay(1000);

  PC.println("=== Test basico ROJO ===");

  // 1. Enviar las letras desordenadas
  STM32_left.println("CHAR:0:OOJR");
  PC.println("→ CHAR:0:OOJR");
  delay(del);

  /*
  // 2. Enviar la palabra original
  STM32.println("WORD:ROJO");
  PC.println("→ WORD:ROJO");
  delay(del);

  // 3. Marcar primera y última
  STM32.println("FIRST");
  PC.println("→ FIRST");
  delay(del);

  STM32.println("ANCHOR");
  PC.println("→ ANCHOR");
  delay(del);

  // 4. Arrancar el juego
  STM32.println("START");
  PC.println("→ START");*/
}

void loop() {
  // Reenviar lo que diga la STM32 hacia el monitor serial
  while (STM32_left.available()) {
    char c = STM32_left.read();
    
    /*PC.write("right: ");
    PC.write(c);
    PC.write("\n");*/

    PC.write(c);
    


  }
}

