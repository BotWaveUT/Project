#include "printf.h"
#include "timer.h"
#include "irq.h"
#include "mini_uart.h"
#include "utils.h"
#include "i2s.h"
#include "dma.h"
#include "input.h"
#include "oscillator.h"

void kernel_main(void)
{
	//uart_init();
	init_printf(0, putc);

	irq_vector_init();
	dma_enable_interrupt();
	enable_irq();

	init_buffer();

	init_phase_inc_table();
	//synth_init();



	pcm_init();		//init PCM
	dma_init();		//init DMA
	pcm_start_transmission();	//PCM TXON enable

	unsigned state = 0;

	//oscillators[0].state = ON;

	while (1){
		//value_keyboard = reading_inputs();
		/*
		if(first_half_empty){

			process_output(&dma_buffer[0]);
			first_half_empty = 0;

		}
		if(second_half_empty){

			process_output(&dma_buffer[DMA_BUFFER_SIZE_HALF]);
			second_half_empty = 0;

		}*/
		if (first_half_empty) {
			process_buffer(&dma_buffer[0], &state);
			first_half_empty = 0;
		}
		else if (second_half_empty) {
			process_buffer(&dma_buffer[DMA_BUFFER_SIZE_HALF], &state);
			second_half_empty = 0;
		}
    }	
}
