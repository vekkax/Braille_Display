/*
 * arq.h
 *
 *  Created on: Sep 27, 2025
 *      Author: santiago
 */

#ifndef INC_ARQ_H_
#define INC_ARQ_H_

#include "stm32c0xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "logic.h"

#define FLAG_REQ_ACK  (1u<<0)
#define FLAG_IS_RETX  (1u<<1)
#define ACK_TIMEOUT_MS 10
enum {
	T_DATA = 0,
	T_ACK,
	T_NAK,
	T_PING,
	T_PONG
};

typedef struct {
    uint8_t msg_type;
    uint8_t src;
    uint8_t dst;
    uint8_t seq;
    uint8_t flags;   // bit0 REQ_ACK, bit1 IS_RETX
} __attribute__((packed)) RHdr;

typedef enum { S_IDLE, S_WAIT_ACK, S_RETX } TxState;

typedef struct {
    TxState st;
    uint8_t next_seq;
    uint8_t pending_seq;
    uint8_t retries_left;
    uint32_t t_expire_ms;

    uint8_t frame[1 + 1 + 2 + MAX_PAYLOAD + 2 + 1]; // max wire
    uint16_t frame_len;
} ArqTx;

ArqTx arq;

void ARQ_Init(void);
uint16_t Build_Data(uint8_t src, uint8_t dst, uint8_t seq, bool req_ack , const uint8_t* user, uint16_t user_len, uint8_t* frame_out, uint16_t cap);
uint16_t Build_ACK(uint8_t src, uint8_t dst, uint8_t seq, uint8_t* frame_out, uint16_t cap);
uint16_t Build_NAK(uint8_t src, uint8_t dst, uint8_t seq, uint8_t* frame_out, uint16_t cap);
bool Handle_Frame(UART_HandleTypeDef* huart_tx, const uint8_t* payload, uint16_t len, uint8_t my_id, uint8_t* txbuf, uint16_t txcap);
void ARQ_Tick(void);
bool ARQ_SendReliable(UART_HandleTypeDef* huart, uint8_t src, uint8_t dst, const uint8_t* user, uint16_t ulen);
void ARQ_OnAckReceived(uint8_t seq);
void ARQ_OnNakReceived(uint8_t seq);


#endif /* INC_ARQ_H_ */
