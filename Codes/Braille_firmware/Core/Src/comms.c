/*
 * comms.c
 *
 *  Created on: Jul 26, 2025
 *      Author: santiago
 */

#include "comms.h"
#include "string.h"
#include "stdlib.h"
#include "main.h"
#include "braille_driver.h"
#include <stdio.h>
#include <stdbool.h>

volatile GameState comms_state = STATE_WAIT_ASSIGNMENT;
volatile CommandType CMD = CMD_UNKNOWN;

volatile char my_letter = '\0';
char original_word[MAX_WORD_LENGTH+1] = {0};
static uint8_t my_index = 0xFF;
static bool is_first = false;
static bool is_anchor = false;
static bool seq_started = false;

static TIM_HandleTypeDef* buzzer_timer = NULL;
static BuzzerState buzzer_state = BEEP_NONE;
static uint32_t buzzer_start_time = 0;

static CircularBuffer circ_left;
static CircularBuffer circ_right;
static uint8_t uart_byte_left;
static uint8_t uart_byte_right;


static UART_HandleTypeDef* uart_left = NULL;
static UART_HandleTypeDef* uart_right = NULL;

static char temp_buffer_left[UART_BUFFER_SIZE] = {0};
static char temp_buffer_right[UART_BUFFER_SIZE] = {0};


// Command Parser
static CommandType ParseCommand(char* msg){
	if (strncmp(msg, "CHAR:", 5) == 0) return CMD_CHAR;
	    if (strncmp(msg, "WORD:", 5) == 0) return CMD_WORD;
	    else if (strncmp(msg, "START", 5) == 0) return CMD_START;
	    else if (strncmp(msg, "RESET", 5) == 0) return CMD_RESET;
	    else if (strncmp(msg, "FIRST", 5) == 0) return CMD_FIRST;
	    else if (strncmp(msg, "ANCHOR", 6) == 0) return CMD_ANCHOR;
	    else return CMD_UNKNOWN;
}

// Buzzer start
void StartBuzzer(BuzzerState state){
	buzzer_state = state;
	buzzer_start_time = __HAL_TIM_GET_COUNTER(buzzer_timer);
	TIM1->CCR1 = 1500; // START Beep
}

// Buzzer update
bool UpdateBuzzer(void){
	uint32_t now = __HAL_TIM_GET_COUNTER(buzzer_timer);
	uint32_t elapsed = (now >= buzzer_start_time) ? now - buzzer_start_time : (0xFFFF - buzzer_start_time + now);

	uint32_t duration = 0;

	switch(buzzer_state){
		case BEEP_NONE:
			return true;
		case BEEP_READY:
			duration = READY_BEEP_TIME;
			break;
		case BEEP_SUCCESS:
			duration = SUCCES_BEEP_TIME;
			break;
		default:
			TIM1->CCR1 = 0;
			buzzer_state = BEEP_NONE;
			return true;
			break;

	}

	if(elapsed >= duration){
		TIM1->CCR1 = 0;
		buzzer_state = BEEP_NONE;
		return true;
	}

	return false;
}


void CircularBuffer_Init(CircularBuffer* cb){
	cb->head=0;
	cb->tail=0;
}

void CircularBuffer_Push(CircularBuffer* cb, char c){
	uint16_t next_head = (cb->head +1) % UART_CIRC_BUFFER_SIZE;
	if (next_head != cb-> tail){
		cb->buffer[cb->head]=c;
		cb->head =next_head;
	}
}

//ReadLine TODO
bool CircularBuffer_ReadLine(CircularBuffer* cb, char* out, size_t max_len) {
    size_t i = 0;
    size_t pos = cb->tail;

    // 1. Busca si hay un '\n' en el buffer
    int found = 0;
    while (pos != cb->head && i < max_len - 1) {
        char c = cb->buffer[pos];
        if (c == '\n') {
            found = 1;
            break;
        }
        pos = (pos + 1) % UART_CIRC_BUFFER_SIZE;
        i++;
    }
    if (!found) return false; // No hay línea completa

    // 2. Copia la línea completa
    i = 0;
    while (cb->tail != cb->head && i < max_len - 1) {
        char c = cb->buffer[cb->tail];
        cb->tail = (cb->tail + 1) % UART_CIRC_BUFFER_SIZE;
        if (c == '\n') break;
        if (c != '\r' && c != '\0')
            out[i++] = c;
    }
    out[i] = '\0';
    return (i > 0);
}


// Initialization
void Comms_Init(UART_HandleTypeDef* huart_left,UART_HandleTypeDef* huart_right, TIM_HandleTypeDef* htim){
	uart_left = huart_left;
	uart_right = huart_right;
	buzzer_timer = htim;

	CircularBuffer_Init(&circ_left);
	CircularBuffer_Init(&circ_right);

	HAL_UART_Receive_IT(uart_left, &uart_byte_left, 1);  // antes: UART_BUFFER_SIZE
	HAL_UART_Receive_IT(uart_right, &uart_byte_right, 1);
}

// Comms processing
void Comms_Process(void){
	switch (comms_state) {

	    case STATE_WAIT_ASSIGNMENT:
	        if (CircularBuffer_ReadLine(&circ_left, temp_buffer_left, UART_BUFFER_SIZE)) {

	            CMD = ParseCommand(temp_buffer_left);

	            switch(CMD) {

	    			case CMD_CHAR: {

						// Buscar los dos puntos
						char* index_str = &temp_buffer_left[5];
						char* colon_ptr = strchr(index_str,':');

						if(colon_ptr != NULL){
							*colon_ptr = '\0';
							uint8_t index = atoi(index_str);

							char* remaining_letters = colon_ptr + 1;

							if(strlen(remaining_letters)>0){
								my_letter = remaining_letters[0];
								my_index = index;
								Braille_Display(my_letter);

								//Resends leftover
								if(strlen(&remaining_letters[1])>0 && uart_right != NULL){
									char forward_msg[UART_BUFFER_SIZE] = {0};
									snprintf(forward_msg,sizeof(forward_msg), "CHAR:%d:%s", index+1,&remaining_letters[1]);
									HAL_UART_Transmit(uart_right, (uint8_t*)forward_msg, strlen(forward_msg), HAL_MAX_DELAY);
								}

							}

						}
						break;
					}

					// WORD COMMAND
	    			case CMD_WORD: {
						strncpy(original_word, &temp_buffer_left[5], MAX_WORD_LENGTH);
						original_word[MAX_WORD_LENGTH] = '\0';

						//Resends WORD
						if (uart_right != NULL) {
								HAL_UART_Transmit(uart_right, (uint8_t*)temp_buffer_left, strlen(temp_buffer_left), HAL_MAX_DELAY);
							}

						break;

					}

	    			// START COMMAND
	    			case CMD_START:{
						if (my_letter != '\0' && original_word[0] != '\0') {
							comms_state = STATE_READY;
						}
						//Resends START
						if (uart_right != NULL) {
								HAL_UART_Transmit(uart_right, (uint8_t*)temp_buffer_left, strlen(temp_buffer_left), HAL_MAX_DELAY);
							}
						break;
	    			}

					// RESET COMMAND
	    			case CMD_RESET :{
						Comms_TriggerReset();

						// Resends RESET
						if (uart_right != NULL) {
							HAL_UART_Transmit(uart_right, (uint8_t*)temp_buffer_left, strlen(temp_buffer_left), HAL_MAX_DELAY);
						}
						break;
	    			}

					// FIRST ASSIGNMENT TODO
	    			case CMD_FIRST:{
							if (original_word[0] == my_letter && my_index == 0) {
								is_first = true;
							}
							if (uart_right != NULL) {
								HAL_UART_Transmit(uart_right, (uint8_t*)"FIRST", 5, HAL_MAX_DELAY);
							}
							break;
	    			}

					// ANCHOR ASSIGNMENT TODO
	    			case CMD_ANCHOR: {
						size_t len = strlen(original_word);
						if (original_word[len - 1] == my_letter && my_index == (len - 1)) {
							is_anchor = true;
						}
						if (uart_right != NULL) {
							HAL_UART_Transmit(uart_right, (uint8_t*)"ANCHOR", 6, HAL_MAX_DELAY);
						}
						break;
	    			}

	    			default:
	    				break;
	            }
	        }
	        break;

	    case STATE_READY:
	        if (buzzer_state == BEEP_NONE) {
	            StartBuzzer(BEEP_READY);
	        } else if (UpdateBuzzer()) {
	            comms_state = STATE_GAME;
	        }
	        break;

	    case STATE_GAME:

	        if (is_first && !seq_started) {
	            char msg[UART_BUFFER_SIZE];
	            snprintf(msg, sizeof(msg), "SEQ:%c", my_letter);
	            HAL_UART_Transmit(uart_right, (uint8_t*)msg, strlen(msg), HAL_TIMEOUT);
	            seq_started = true;
	        }

	        // Procesar datos recibidos por la izquierda
	        if (CircularBuffer_ReadLine(&circ_left, temp_buffer_left,UART_BUFFER_SIZE)){

	            if (strncmp(temp_buffer_left, "SEQ:", 4) == 0) {
	                char* seq = &temp_buffer_left[4];
	                char new_seq[UART_BUFFER_SIZE];
	                snprintf(new_seq, sizeof(new_seq), "%.*s%c", (int)(sizeof(new_seq) - 2), seq, my_letter);

	                if (is_anchor && strcmp(new_seq, original_word) == 0) {
	                    HAL_UART_Transmit(uart_left, (uint8_t*)"WIN", 3, HAL_TIMEOUT);
	                    comms_state = STATE_SUCCESS;
	                } else {
	                    char msg[UART_BUFFER_SIZE];
	                    snprintf(msg, sizeof(msg), "SEQ:%.*s", (int)(sizeof(msg) - 5), new_seq);
	                    HAL_UART_Transmit(uart_right, (uint8_t*)msg, strlen(msg), HAL_TIMEOUT);
	                }
	            }

	            else if (strncmp(temp_buffer_left, "WIN", 3) == 0) {
	                Braille_Display(' ');
	                comms_state = STATE_SUCCESS;
	                HAL_UART_Transmit(uart_right, (uint8_t*)temp_buffer_left, strlen(temp_buffer_left), HAL_TIMEOUT);
	            }
	        }

	        // Procesar datos recibidos por la derecha
	        if (CircularBuffer_ReadLine(&circ_right, temp_buffer_right,UART_BUFFER_SIZE)){

	            if (strncmp(temp_buffer_right, "WIN", 3) == 0) {
	                Braille_Display(' ');
	                comms_state = STATE_SUCCESS;
	                HAL_UART_Transmit(uart_left, (uint8_t*)temp_buffer_right, strlen(temp_buffer_right), HAL_TIMEOUT);
	            }
	        }

	        break;

	    case STATE_VERIFY:
	        // (Optional intermediate state)
	        break;

	    case STATE_SUCCESS:
	        if (buzzer_state == BEEP_NONE) {
	            StartBuzzer(BEEP_SUCCESS);
	        } else if (UpdateBuzzer()) {
	            comms_state = STATE_SHUTDOWN;
	        }
	        break;

	    case STATE_SHUTDOWN:
	        // Ignore everything unless RESET arrives
	        if (CircularBuffer_ReadLine(&circ_left, temp_buffer_left,UART_BUFFER_SIZE)) {
	            if (strncmp(temp_buffer_left, "RESET", 5) == 0) {
	                Comms_TriggerReset();
	            }
	        }
	        break;
	    }
}

// --- Manejador de recepción por interrupción ---
void Comms_OnUARTReceive(UART_HandleTypeDef* huart) {
	 if (huart == uart_left) {
		CircularBuffer_Push(&circ_left, uart_byte_left);
		HAL_UART_Receive_IT(uart_left, &uart_byte_left, 1);

	} else if (huart == uart_right) {
		CircularBuffer_Push(&circ_right, uart_byte_right);
		HAL_UART_Receive_IT(uart_right, &uart_byte_right, 1);
	}
}

// --- Reset manual ---
void Comms_TriggerReset(void) {
    comms_state = STATE_WAIT_ASSIGNMENT;
    my_letter = '\0';
    original_word[0] = '\0';
}


