#include "dma.h"
#include "i2s.h"
#include "peripherals/irq.h"
#include "oscillator.h"

static struct Control_Block DMA_BLOCK_1;
static struct Control_Block DMA_BLOCK_2;

unsigned dma_buffer[DMA_BUFFER_SIZE] = {0};

void init_buffer() {
    for (int i = 0; i < DMA_BUFFER_SIZE; i++) {
        dma_buffer[i] = 0;
    }
}

void dma_enable_interrupt() {
    *(volatile unsigned*)ENABLE_IRQS_1 |= (1 << 17); //enable IRQ for DMA CANAL 1


}

void handler_dma_interrupt() {
    if (*(volatile unsigned*)DMA_CONBLK_AD_1 == (unsigned)((unsigned long)(&DMA_BLOCK_1) | 0xC0000000)) {
        second_half_empty = 1;
    } else if (*(volatile unsigned*)DMA_CONBLK_AD_1 == (unsigned)((unsigned long)(&DMA_BLOCK_2) | 0xC0000000)){
        first_half_empty = 1;
    }

    *(volatile unsigned*)DMA_CS_1 |= (1 << 2);
    
}

void process_buffer(unsigned* buffer, unsigned* pstate) {
    for (unsigned i = 0; i < DMA_BUFFER_SIZE_HALF; i++) {
        if (*pstate < 50)
            buffer[i] = 0x30003000;
        else
            buffer[i] = 0x0;
        (*pstate)++;

        if (*pstate >= 100) {
            (*pstate) = 0;
        }
    }
    
}


void dma_init_controlBlock() {
    DMA_BLOCK_1.TI = (2 << 16) | (1 << 8) | (1 << 6) | (1 << 3) | 1; //set PERMAP TO PCM TX | increment source addrr after read | wait PCM to send data to write
    DMA_BLOCK_1.SOURCE_AD = (unsigned)((unsigned long)(&dma_buffer[0]) | 0xC0000000);
    DMA_BLOCK_1.DEST_AD = (unsigned)PCM_FIFO_TX_BUS;
    DMA_BLOCK_1.TXFR_LEN = DMA_BUFFER_SIZE_HALF * 4;
    DMA_BLOCK_1.STRIDE = 0;
    DMA_BLOCK_1.NEXTCONBK = (unsigned)((unsigned long)(&DMA_BLOCK_2) | 0xC0000000);

    DMA_BLOCK_2.TI = (2 << 16) | (1 << 8) | (1 << 6) | (1 << 3) | 1; //set PERMAP TO PCM TX | increment source addrr after read
    DMA_BLOCK_2.SOURCE_AD = (unsigned)((unsigned long)(&dma_buffer[DMA_BUFFER_SIZE_HALF]) | 0xC0000000);
    DMA_BLOCK_2.DEST_AD =  (unsigned)PCM_FIFO_TX_BUS;
    DMA_BLOCK_2.TXFR_LEN = DMA_BUFFER_SIZE_HALF * 4;
    DMA_BLOCK_2.STRIDE = 0;
    DMA_BLOCK_2.NEXTCONBK = (unsigned)((unsigned long)(&DMA_BLOCK_1) | 0xC0000000);
}

/**
 * @brief Init chanel 0 of DMA and set Control block DMA_BLOCK for the chanel 0
 * 
 */
void dma_init() {
    *(volatile unsigned*)DMA_ENABLE_REG = (1 << 1);    //enable DMA engine chanel 0
    dma_init_controlBlock();

    *(volatile unsigned*)DMA_CS_1 = (1 << 31); //RESET DMA CHANEL 0 to clear

    while(*(volatile unsigned*)DMA_CS_1 & (1 << 31)); //wait the bit set at 0

    *(volatile unsigned*)DMA_CONBLK_AD_1 = (unsigned)((unsigned long)(&DMA_BLOCK_1) | 0xC0000000);
    *(volatile unsigned*)DMA_CS_1 = 1; //enable DMA chanel 0
}