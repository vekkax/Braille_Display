/*
 * uart_test.c
 *
 *  Created on: Oct 22, 2025
 *      Author: santiago
 */

#include "uart_test.h"
#include <string.h>

static const char ping_str[] = "ping";
static const char pong_str[] = "pong\r\n";

void UART_Test_Init(UART_TestHandle *h, UART_HandleTypeDef *huart) {
    h->huart = huart;
    h->rx_byte = 0;
    HAL_UART_Receive_IT(huart, &h->rx_byte, 1);
}

void UART_Test_RxCallback(UART_TestHandle *h) {
    static char buffer[8];
    static uint8_t idx = 0;

    buffer[idx++] = h->rx_byte;

    // wrap buffer
    if (idx >= sizeof(buffer)) idx = 0;

    // look for "ping"
    if (idx >= 4) {
        if (memcmp(&buffer[idx - 4], ping_str, 4) == 0) {
            HAL_UART_Transmit(h->huart, (uint8_t *)pong_str, strlen(pong_str), 100);
        }
    }

    // re-arm interrupt
    HAL_UART_Receive_IT(h->huart, &h->rx_byte, 1);
}

void UART_Test_Process(UART_TestHandle *h) {
    // nothing required for now, left for future extensions
}

