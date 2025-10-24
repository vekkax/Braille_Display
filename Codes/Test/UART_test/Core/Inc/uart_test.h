/*
 * uart_test.h
 *
 *  Created on: Oct 22, 2025
 *      Author: santiago
 */

#ifndef INC_UART_TEST_H_
#define INC_UART_TEST_H_

#include "stm32c0xx_hal.h"  // or your specific series

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    UART_HandleTypeDef *huart;
    uint8_t rx_byte;
} UART_TestHandle;

/**
 * @brief Initialize UART test handle (starts reception interrupt)
 */
void UART_Test_Init(UART_TestHandle *h, UART_HandleTypeDef *huart);

/**
 * @brief Call this from HAL_UART_RxCpltCallback()
 */
void UART_Test_RxCallback(UART_TestHandle *h);

/**
 * @brief Optional periodic process function (not required)
 */
void UART_Test_Process(UART_TestHandle *h);

#ifdef __cplusplus
}
#endif


#endif /* INC_UART_TEST_H_ */
