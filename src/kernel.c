#include "dma.h"
#include "i2s.h"
#include "input.h"
#include "irq.h"
#include "mini_uart.h"
#include "oscillator.h"
#include "printf.h"
#include "timer.h"
#include "utils.h"

void kernel_main(void) {
    // uart_init();
    init_printf(0, putc);

    irq_vector_init();
    dma_enable_interrupt();
    enable_irq();

    init_buffer();

    init_phase_inc_table();
    synth_init();

    pcm_init();               // init PCM
    dma_init();               // init DMA
    pcm_start_transmission(); // PCM TXON enable

    Init_Pin_Keyboard();
    init_led();

    while (1) {
        
        

        if (first_half_empty) {
            read_buttons();
            process_output(&dma_buffer[0]);
            first_half_empty = 0;
        }
        if (second_half_empty) {
            read_buttons();
            process_output(&dma_buffer[DMA_BUFFER_SIZE_HALF]);
            second_half_empty = 0;
        }
    }
}
