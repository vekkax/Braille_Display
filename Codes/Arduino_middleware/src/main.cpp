// Master.ino
#include <Arduino.h>
#include "arduino_hal_shim.h"
extern "C" {
  #include "frame.h"
  #include "arq.h"
}

// --------- Configuración ---------
static UART_HandleTypeDef huart_chain;   // Serial1 hacia la cadena
static const uint32_t CHAIN_BAUD = 115200; // ajústalo a tu baud real

// Buffers y parser para RX desde cadena
static CircularBuffer circ_chain;
static uint8_t fp_out[UART_BUFFER_SIZE];
static FrameParser fp_chain;

// Estado del master
static uint8_t ready_count = 0;
static bool game_started = false;

// ---------- Utils ----------
static inline void logPC(const char* s){ Serial.println(s); }

static bool send_reliable_str(const char* s){
  return ARQ_SendReliable(&huart_chain, (const uint8_t*)s, (uint16_t)strlen(s));
}

// ---------- Arranque ----------
void setup(){
  // PC <-> Master
  Serial.begin(115200);
  while(!Serial){}

  // Cadena <-> Master
  Serial1.begin(CHAIN_BAUD);
  huart_chain.ser  = reinterpret_cast<void*>(&Serial1);
  huart_chain.baud = CHAIN_BAUD;

  CircularBuffer_Init(&circ_chain);
  FP_Init(&fp_chain, fp_out, sizeof(fp_out));
  ARQ_Init();

  logPC("READY Master");
  logPC("Use commands: SET WORD HOLA | SET INDEX 2301 | SEND START | SEND RESET | STATUS");
}

// ---------- RX desde la cadena ----------
static void pump_chain_rx(){
  // 1) poll de Serial1 -> circular buffer
  while (Serial1.available() > 0){
    uint8_t b = (uint8_t)Serial1.read();
    CircularBuffer_Push(&circ_chain, b);
  }

  // 2) parse frames y pásalas por ARQ
  while (true){
    FrameParserState st = FP_Tick(&fp_chain, &circ_chain);
    if (st == NO_DATA) break;
    if (st == GOOD_FRAME){
      ArqInd ind;
      ArqEvent ev = Handle_Frame(&huart_chain, fp_chain.out, fp_chain.len, true,
                                 (uint8_t*)arq.frame, sizeof(arq.frame), &ind);
      if (ev == ARQ_EVT_DATA && ind.user && ind.user_len){
        // Mensaje de aplicación (ASCII): WORD/INDEX/READY/START/SEQ/WIN/RESET
        // Repórtalo al PC y procesa lo relevante para el master
        String s; s.reserve(ind.user_len+1);
        for (uint16_t i=0;i<ind.user_len;i++) s += (char)ind.user[i];
        Serial.print("RX_CHAIN "); Serial.println(s);

        if      (s.startsWith("READY")){
          ready_count++;
          Serial.print("EVENT READY_COUNT "); Serial.println(ready_count);
          if (ready_count >= 4 && !game_started){
            send_reliable_str("START");
            Serial.println("EVENT START_SENT");
            game_started = true;
          }
        }
        else if (s.startsWith("WIN")){
          Serial.println("EVENT WIN");
        }
        else if (s.startsWith("RESET")){
          // si un módulo reinicia, resetea conteo y estado
          ready_count = 0;
          game_started = false;
          Serial.println("EVENT CHAIN_RESET_SEEN");
        }
      }
      FP_Init(&fp_chain, fp_chain.out, fp_chain.out_cap); // listo para el siguiente
    }
  }

  // 3) timers ARQ (reintentos)
  ARQ_Tick();
}

// ---------- Parser de comandos desde PC ----------
static void handle_pc_line(const String& line){
  // Formato simple:
  //  SET WORD HOLA
  //  SET INDEX 2301
  //  SEND START
  //  SEND RESET
  //  STATUS

  if (line.startsWith("SET WORD ")){
    String w = line.substring(9);
    w.trim();
    if (w.length() != 4){ Serial.println("ERR WORD len!=4"); return; }
    char cmd[16]; snprintf(cmd, sizeof(cmd), "WORD:%s", w.c_str());
    if (send_reliable_str(cmd)) Serial.println("OK WORD_SENT");
    else Serial.println("ERR WORD_SEND");
  }
  else if (line.startsWith("SET INDEX ")){
    String idx = line.substring(10);
    idx.trim();
    if (idx.length() != 4){ Serial.println("ERR INDEX len!=4"); return; }
    char cmd[16]; snprintf(cmd, sizeof(cmd), "INDEX:%s", idx.c_str());
    if (send_reliable_str(cmd)) Serial.println("OK INDEX_SENT");
    else Serial.println("ERR INDEX_SEND");
  }
  else if (line == "SEND START"){
    if (send_reliable_str("START")){ Serial.println("OK START_SENT"); game_started=true; }
    else Serial.println("ERR START_SEND");
  }
  else if (line == "SEND RESET"){
    if (send_reliable_str("RESET")){
      Serial.println("OK RESET_SENT");
      ready_count = 0; game_started=false;
    } else Serial.println("ERR RESET_SEND");
  }
  else if (line == "STATUS"){
    Serial.print("STATUS ready_count="); Serial.print(ready_count);
    Serial.print(" game_started="); Serial.println(game_started ? "1":"0");
  }
  else {
    Serial.println("ERR UNKNOWN_CMD");
  }
}

// ---------- Bucle ----------
void loop(){
  // 1) Procesa RX/ARQ con la cadena
  pump_chain_rx();

  // 2) Lee líneas desde PC y actúa
  static String acc;
  while (Serial.available() > 0){
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r'){
      acc.trim();
      if (acc.length()) handle_pc_line(acc);
      acc = "";
    } else {
      acc += c;
    }
  }
}
