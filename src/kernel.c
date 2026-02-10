#include "printf.h"
#include "timer.h"
#include "irq.h"
#include "mini_uart.h"
#include "utils.h"
#include "i2s.h"
#include "dma.h"


void kernel_main(void)
{
	//uart_init();
	init_printf(0, putc);

	irq_vector_init();
	generic_timer_init();
	enable_interrupt_controller();
	disable_irq();

	generate_square_signal();

	enable_I2S();
	dma_init();
	pcm_start_transmission();

	while (1){
	}
		
}
