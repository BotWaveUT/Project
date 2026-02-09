#include "printf.h"
#include "timer.h"
#include "irq.h"
#include "mini_uart.h"
#include "utils.h"
#include "i2s.h"


void kernel_main(void)
{
	//uart_init();
	init_printf(0, putc);

	irq_vector_init();
	generic_timer_init();
	enable_interrupt_controller();
	disable_irq();
	init_I2S();

	while (1){
	/*
		if (i < 32)
			send_data_to_pcm(0xFFFFFFFF);
		else {
			send_data_to_pcm(0);
			if (i == 64)
				i = 0;
		}
		i++;*/
		//uart_send('A');
		send_data_to_pcm();
	}
		
}
