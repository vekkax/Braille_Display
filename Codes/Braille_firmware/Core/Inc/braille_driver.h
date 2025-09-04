/*
 * electromagnet_driver.h
 *
 *  Created on: Jul 26, 2025
 *      Author: santiago
 */

#ifndef INC_BRAILLE_DRIVER_H_
#define INC_BRAILLE_DRIVER_H_

#include "stm32c0xx_hal.h"
#include <stdint.h>

#define BRAILLE_DOT_COUNT 6
#define BRAILLE_ACTIVE_TIME_MS 500

typedef enum{
	BRAILLE_A = 0,
	BRAILLE_B,
	BRAILLE_C,
	BRAILLE_D,
	BRAILLE_E,
	BRAILLE_F

} BrailleDot_t;

//debug function
void Braille_DebugUpdateAllPins(void);

//init function
void Braille_Init(TIM_HandleTypeDef* htim);


//main use functions
void Braille_Update(void);
void Braille_Display(char letter);


#endif /* INC_BRAILLE_DRIVER_H_ */
