#ifndef	_P_GPIO_H
#define	_P_GPIO_H

#include "peripherals/base.h"

#define GPFSEL1         (PBASE+0x00200004)
#define GPFSEL2         (PBASE+0x00200008)
#define GPSET0          (PBASE+0x0020001C)
#define GPCLR0          (PBASE+0x00200028)
#define GPPUD           (PBASE+0x00200094)
#define GPPUDCLK0       (PBASE+0x00200098)
#define GPAREN0         (PBASE+0x0020007C)  //Asynchronous input
#define GPEDS0          (PBASE+0x00200040)  //Event on pin

#endif  /*_P_GPIO_H */
