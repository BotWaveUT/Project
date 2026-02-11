#ifndef _I2S_H
#define _I2S_H

#include "peripherals/gpio.h"

#define PCM_BASE         (PBASE + 0x203000)
#define GPIO_BASE        (PBASE + 0x200000)
#define CM_PCMCTL        (PBASE + 0x101098)
#define CM_PCMDIV        (PBASE + 0x10109c)

/* PCM */
#define PCM_CS_A    (PCM_BASE + 0x00)
#define PCM_FIFO_A  (PCM_BASE + 0x04)
#define PCM_MODE_A  (PCM_BASE + 0x08)
#define PCM_TXC_A   (PCM_BASE + 0x10)
#define PCM_DREQ_A  (PCM_BASE + 0x14)


#define CM_PASSWORD 0x5A000000
#define BUSY        7

void pcm_init();
void pcm_start_transmission();


#endif //_I2S_H
