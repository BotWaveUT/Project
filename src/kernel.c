#include "printf.h"
#include "timer.h"
#include "irq.h"
#include "mini_uart.h"
#include "utils.h"
#include "i2s.h"
#include "dma.h"
#include "input.h"


void kernel_main(void)
{
	//uart_init();
	init_printf(0, putc);

	irq_vector_init();
	generic_timer_init();
	enable_interrupt_controller();
	disable_irq();

	input_init();	//init buttons of input
	init_buffer();	//set the DMA buffer for PCM

	pcm_init();		//init PCM
	dma_init();		//init DMA
	pcm_start_transmission();	//PCM TXON enable

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
