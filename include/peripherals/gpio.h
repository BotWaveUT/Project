#ifndef	_P_GPIO_H
#define	_P_GPIO_H

#include "peripherals/base.h"

#define GPFSEL1         (PBASE+0x00200004)
#define GPFSEL2         (PBASE+0x00200008)
#define GPSET0          (PBASE+0x0020001C)
#define GPCLR0          (PBASE+0x00200028)
#define GPPUD           (PBASE+0x00200094)
#define GPPUDCLK0       (PBASE+0x00200098)
#define GPREN0          (PBASE+0x0020004C)
#define GPREN1          (PBASE+0x00200050)
#define GPLEV0          (PBASE+0x00200034)
#define GPLEV1          (PBASE+0x00200038)


#endif  /*_P_GPIO_H */
