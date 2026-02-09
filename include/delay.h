

#ifndef __delay__H__
#define __delay__H__

#include "stdint.h"

/*
 * @brief This function will initialize SysTick
 * to generate 1ms interrupt.
 * @param freq source frequency of SysTick.
 * @return nothing.
 * @see Generic ARM CortexM4 user guide.
 * */

void delay_init(uint32_t freq);

/*
 * @brief This function will return the current millis.
 *
 * @param nothing.
 * @return current millis.
 * */
uint64_t millis();


/*
 * @brief This function will spin lock the CPU to delay for the required
 * amount
 * @param time to be delayed in milliseconds.
 * @return nothing.
 * */
void delay(uint32_t time);


#endif /* DELAY_H_ */
