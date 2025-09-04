/*
 * comms.h
 *
 *  Created on: Jul 26, 2025
 *      Author: santiago
 */

#ifndef INC_COMMS_H_
#define INC_COMMS_H_

#include "stm32c0xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_WORD_LENGTH 4
#define UART_BUFFER_SIZE 16
#define UART_CIRC_BUFFER_SIZE 64

#define UART_LEFT huart2
#define UART_RIGHT huart1

#define READY_BEEP_TIME 200
#define SUCCES_BEEP_TIME 500

typedef enum {
	STATE_WAIT_ASSIGNMENT,
	STATE_READY,
	STATE_GAME,
	STATE_VERIFY,
	STATE_SUCCESS,
	STATE_SHUTDOWN
}GameState;

typedef struct {
	char buffer[UART_CIRC_BUFFER_SIZE];
	volatile uint16_t head;
	volatile uint16_t tail;
}CircularBuffer;

typedef enum {
    CMD_UNKNOWN = 0,
    CMD_CHAR,
    CMD_WORD,
    CMD_START,
    CMD_RESET,
    CMD_FIRST,
    CMD_ANCHOR
} CommandType;

typedef enum{
	BEEP_NONE,
	BEEP_READY,
	BEEP_SUCCESS,
}BuzzerState;

extern volatile GameState comms_state;
extern volatile char my_letter;
extern char original_word[MAX_WORD_LENGTH+1];
extern char rx_buffer_left[UART_BUFFER_SIZE];
extern char rx_buffer_right[UART_BUFFER_SIZE];


void Comms_Init(UART_HandleTypeDef* huart_left, UART_HandleTypeDef* huart_right, TIM_HandleTypeDef* htim);
void Comms_Process(void);
void Comms_OnUARTReceive(UART_HandleTypeDef* huart);
void Comms_TriggerReset(void);

void CicularBuffer_Init(CircularBuffer* cb);
bool CircularBuffer_ReadLine(CircularBuffer* cb, char* out, size_t max_len);
void CircularBuffer_Push(CircularBuffer* cb, char c);


#endif /* INC_COMMS_H_ */
