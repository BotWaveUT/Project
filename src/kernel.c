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

	init_buffer();

	enable_I2S();
	dma_init();
	pcm_start_transmission();

	unsigned alt_buffer = 0;
	unsigned state = 0;
	while (1){
		if (get_free_buffer() == 0 && alt_buffer == 0) {
			process_buffer(&dma_buffer[0], &state);
			alt_buffer = 1;
		}
		else if (get_free_buffer() == 1 && alt_buffer == 1) {
			process_buffer(&dma_buffer[DMA_BUFFER_SIZE_HALF], &state);
			alt_buffer = 0;
		}
	}
		
}
