// arduino_hal_shim.h
#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------- Minimal HAL-like types -----------------
typedef struct {
    void* ser;      // holds a pointer to HardwareSerial (opaque to C)
    uint32_t baud;
} UART_HandleTypeDef;

typedef enum { HAL_OK = 0, HAL_ERROR = 1 } HAL_StatusTypeDef;


uint32_t HAL_GetTick(void);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef* huart,
                                    uint8_t* data, uint16_t len,
                                    uint32_t timeout_ms);

// Algunas macros que puedan usar tus .c
#define __attribute__(x)
#ifdef __cplusplus
}
#endif
