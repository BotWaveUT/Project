#ifndef __DMA_H__
#define __DMA_H__

#include "peripherals/base.h"

#define DMA_BUFFER_SIZE  2048
#define DMA_BUFFER_SIZE_HALF 1024


#define DMA_BASE            (PBASE + 0x00007000)
#define DMA_CHANEL_1        (DMA_BASE + 0x100)

#define DMA_CS_1            (DMA_CHANEL_1 + 0x0)
#define DMA_CONBLK_AD_1     (DMA_CHANEL_1 + 0x4)
#define DMA_TI_1            (DMA_CHANEL_1 + 0x8)
#define DMA_SOURCE_AD_1     (DMA_CHANEL_1 + 0xc)
#define DMA_DEST_AD_1       (DMA_CHANEL_1 + 0x10)
#define DMA_TXFR_LEN_1      (DMA_CHANEL_1 + 0x14)
#define DMA_STRIDE_1        (DMA_CHANEL_1 + 0x18)
#define DMA_NEXTCONBK_1     (DMA_CHANEL_1 + 0x1c)
#define DMA_DEBUG_1         (DMA_CHANEL_1 + 0x20)

#define DMA_ENABLE_REG      (DMA_BASE + 0xFF0)


#define PCM_FIFO_TX_BUS     (0x7E203000 + 0x4)

//aligned at 8 words (256 bits)
struct Control_Block {
    unsigned TI;                //transfer information
    unsigned SOURCE_AD;         //source address
    unsigned DEST_AD;           //destination address
    unsigned TXFR_LEN;          //transfer length
    unsigned STRIDE;            //2D mode stride
    unsigned NEXTCONBK;         //Next control block address
    unsigned PADDING[2];        //useless but needed
}__attribute__( (aligned(32)) );

extern unsigned dma_buffer[DMA_BUFFER_SIZE];

void dma_init();
void init_buffer();
int get_free_buffer();
void process_buffer(unsigned* buffer, unsigned* pstate);

#endif //__DMA_H__