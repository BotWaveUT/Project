#ifndef __INPUT_H__
#define __INPUT_H__

#include <stdint.h>

#define BUTT_DO 2
#define BUTT_RE 5
#define BUTT_MI 8
#define BUTT_FA 11
#define BUTT_SOL 14
#define BUTT_LA 17
#define BUTT_SI 20

#define BUTT_OCT_INC 30
#define BUTT_OCT_DEC 31

void input_init();
int is_button_pressed();
void Init_Pin_Keyboard();
uint64_t reading_inputs();

void init_led();

void led_on();
void led_off() ;

#endif