/*
 * keyboard.c
 *
 *  Created on: Feb 4, 2026
 *      Author: Tina
 */

#include "keyboard.h"
#include "printf.h"

// i range macros
#define MASK(l) ((1 << (l)) - 1)
#define GET_iS(x, i, l) (((x) >> (i)) & MASK(l))
#define REP_iS(x, i, l, y) (((x) & ~(MASK(l) << i)) | ((y) << (i)))
#define GPIO_MODER_IN 0b00
#define GPIO_MODER_OUT 0b01
#define GPIO_PUPDR_PD 0b10
#define GPIO_PUPDR_NO 0b00
#define GPIO_MODER_ANA 0b11

#define RCC_TIM4EN (1 << 2)

// GPIOB
#define SH_LD 0  // entrée
#define CLK 1    // entrée
#define SERIAL 2 // sortie

#define NB_BUTT 8

void delay_tick(volatile int d) {
    while (d--)
        ;
}

void Init_Pin_Keyboard() {

}

uint64_t normalize_keyboard(uint64_t raw) {
    uint64_t v = 0;
    for (int j = 0; j < 5; j++) {

        for (int i = 0; i < 8; i++) {
            int raw_i = j * 8 + i;
            int norm_i = j * 8 + (7 - i);

            if (raw & (1ULL << raw_i))
                v |= (1ULL << norm_i);
        }
    }
    return v;
}

uint64_t reading_inputs(void) {
    uint64_t v = 0;

    uint64_t v1 = normalize_keyboard(v);
    return v1;
}
