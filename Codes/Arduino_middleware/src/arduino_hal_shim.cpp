#include <Arduino.h>
#include "arduino_hal_shim.h"

// Ensure C linkage for the symbols used by C files
extern "C" {

uint32_t HAL_GetTick(void){
    return millis();
}

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef* huart,
                                    uint8_t* data, uint16_t len,
                                    uint32_t /*timeout_ms*/) {
    if (!huart || !huart->ser) return HAL_ERROR;
    // Cast back to HardwareSerial*
    auto* s = reinterpret_cast<HardwareSerial*>(huart->ser);
    size_t n = s->write(data, len);
    s->flush();
    return (n == len) ? HAL_OK : HAL_ERROR;
}

} // extern "C"
