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

    unsigned state = 0;

    Init_Pin_Keyboard();
    init_led();

    // oscillators[3].state = ON;
    // oscillators[3].amplitude = 1.f;

    // oscillators[4].state = ON;
    // oscillators[4].amplitude = 1.f;

    // oscillators[5].state = ON;
    // oscillators[5].amplitude = 1.f;

    // oscillators[6].state = ON;
    // oscillators[6].amplitude = 1.f;

    

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
