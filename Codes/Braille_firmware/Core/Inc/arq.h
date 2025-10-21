/*
 * arq.h
 *
 *  Created on: Sep 27, 2025
 *      Author: santiago
 */

#ifndef INC_ARQ_H_
#define INC_ARQ_H_

#include "stm32c0xx_hal.h"   // adjust to your MCU family
#include "frame.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ----- Frame/header constants you already use -----
#ifndef MAX_PAYLOAD
#define MAX_PAYLOAD 32
#endif

// Flags for RHdr.flags
typedef enum {
    FLAG_NONE    = 0,
    FLAG_REQ_ACK = (1u << 0),
    FLAG_IS_RETX = (1u << 1),
} FrameFlags;

// Message types
typedef enum {
    T_DATA = 0,
    T_ACK,
    T_NAK,
    T_PING,
    T_PONG
} MsgType;

// 5-byte reliable header at the start of PAYLOAD
typedef struct {
    uint8_t msg_type;   // DATA / ACK / NAK / PING / PONG
    uint8_t seq;        // sequence number for reliability
    uint8_t flags;      // bitmask: FLAG_REQ_ACK, FLAG_IS_RETX, etc.
} __attribute__((packed)) RHdr;

// TX ARQ state
typedef enum { S_IDLE, S_WAIT_ACK, S_RETX } TxState;

#ifndef ACK_TIMEOUT_MS
#define ACK_TIMEOUT_MS 10u   // tune as needed
#endif

typedef struct {
    TxState   st;
    UART_HandleTypeDef* huart_tx;   // << store which UART to use
    uint8_t  next_seq;
    uint8_t  pending_seq;
    uint8_t  retries_left;
    uint32_t t_expire_ms;
    uint8_t  frame[1 + 1 + 2 + MAX_PAYLOAD + 2 + 1]; // full wire buffer
    uint16_t frame_len;
} ArqTx;


typedef enum { ARQ_EVT_NONE=0, ARQ_EVT_DATA, ARQ_EVT_ACK, ARQ_EVT_NAK, ARQ_EVT_PING, ARQ_EVT_PONG } ArqEvent;

typedef struct {
    uint8_t  msg_type;   // same as your header
    uint8_t  seq;
    uint8_t  flags;
    const uint8_t* user; // only valid for ARQ_EVT_DATA
    uint16_t user_len;
} ArqInd;


// Global context (keep if you want a single channel)
extern ArqTx arq;

// ---- Public API ----
void ARQ_Init(void);

// Build frames that include your on-wire format (SYNC..LEN..PAYLOAD..CRC..END)
uint16_t Build_Data( uint8_t seq, bool req_ack,const uint8_t* user, uint16_t user_len, uint8_t* frame_out, uint16_t cap);
uint16_t Build_ACK (uint8_t seq, uint8_t* frame_out, uint16_t cap);
uint16_t Build_NAK ( uint8_t seq, uint8_t* frame_out, uint16_t cap);

// auto_ack: if true, ARQ replies with ACK when needed; if false, you’ll do it.
ArqEvent Handle_Frame(UART_HandleTypeDef* huart_tx, const uint8_t* payload, uint16_t len, bool auto_ack, uint8_t* txbuf, uint16_t txcap, ArqInd* out);

// ARQ sender (stop-and-wait)
bool ARQ_SendReliable(UART_HandleTypeDef* huart, const uint8_t* user, uint16_t ulen);
void ARQ_Tick(void);

// RX callbacks from your logic/parser
void ARQ_OnAckReceived(uint8_t seq);
void ARQ_OnNakReceived(uint8_t seq);

// Optional (only used if you use PING/PONG)
void ARQ_OnPong(uint8_t seq);


#endif /* INC_ARQ_H_ */
