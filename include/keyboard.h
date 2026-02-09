/*
 * keyboard.h
 *
 *  Created on: Feb 4, 2026
 *      Author: Tina
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_
#include <stdint.h>

void Init_Pin_Keyboard() ;
uint64_t reading_inputs();
void debug_read_raw(void);

#endif /* KEYBOARD_H_ */


