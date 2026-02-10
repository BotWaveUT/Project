#include "i2s.h"
#include "utils.h"
#include "printf.h"
#include "mini_uart.h"
#include "math.h"




static void pcm_fill_fifo() {
    for (unsigned i = 0; i < 64; i++) {
            *(volatile unsigned*)PCM_FIFO_A = 0x8000800; 
    }
}


static void pcm_init_gpio()
{
    unsigned int FSEL1 = *(volatile unsigned*)GPFSEL1; //GPIO 18 - 19
    unsigned int FSEL2 = *(volatile unsigned*)GPFSEL2; //GPIO 20 - 21

    FSEL1 &= ~(((0b111) << 24) | ((0b111) << 27));
    FSEL2 &= ~((0b111) | ((0b111) << 3));

    FSEL1 |= ((0b100 << 24) | (0b100 << 27));
    FSEL2 |= ((0b100) | (0b100 << 3));

    *(volatile unsigned*)GPFSEL1 = FSEL1;
    *(volatile unsigned*)GPFSEL2 = FSEL2;
    
    *(volatile unsigned*)GPPUD = 0;
    delay(150);
    *(volatile unsigned*)GPPUDCLK0 = (1 << 18 | 1 << 19 | 1 << 20 | 1 << 21);
    delay(150);
    *(volatile unsigned*)GPPUDCLK0 = 0;
}

/**
 * @brief Function to set the clock for PCM with PLLD
 * PLLD = 500 MHz
 * BCLK = 44100 * 2 * 16
 * 500 000 000 / (44100 * 2 * 16) = 354.30839
 * 
 * 054 = 0x162
 * 0.30839 * 2^12 = 1263
 * 
 */
static void pcm_init_clock() {
    *(volatile unsigned*)CM_PCMCTL = (CM_PASSWORD | (*(volatile unsigned*)CM_PCMCTL & (~ 0x10)));  //disable clock

    while(*(volatile unsigned*)CM_PCMCTL & (1 << BUSY));    //wait the clock is not busy

    *(volatile unsigned*)CM_PCMDIV = (CM_PASSWORD | (0x162 << 12) | 0x4EF); //set the divider

    *(volatile unsigned*)CM_PCMCTL = (CM_PASSWORD) | (0b01 << 9) | 0x10 | 0b0110; //set the clock with 1 division, PLLD used and enable
}


static void pcm_init_modea()
{
    unsigned mode_a = 0;
    mode_a &= (~(1 << 28));   //enable PCM clock 
    mode_a |= (1 << 24);    //split fifo word into 2 words of 16bits
    mode_a &= (~(1 << 23));   //master mode PCM CLK to output
    mode_a &= (~(1 << 21));   //master mode FS CLK to ouput
    mode_a &= (~(1 << 20));    //FS is high first then low
    mode_a |= (31 << 10);   //nbr PCM clock master for 1 word
    mode_a |= (16);         //nbr PCM clock for FS word
    
    *(volatile unsigned*)PCM_MODE_A = mode_a;
}

static void pcm_init_tx() {
    unsigned tx_a = 0;

    tx_a |= (1 << 30);  //enable chanel 1
    tx_a |= (8 << 16);   //width of 16bits for chanel 1
    tx_a |= (0 << 20);  //position 0

    tx_a |= (1 << 14); //enable chanel 2
    tx_a |= (8);   //width of 16bits for chanel 2
    tx_a |= (16 << 4);  //position of the value to chanel 2 to send

    *(volatile unsigned*)PCM_TXC_A = tx_a;
}

void enable_I2S(){
    pcm_init_clock();
    pcm_init_gpio();
    *(volatile unsigned*)PCM_DREQ_A = (32 << 8); //half of the PCM FIFO request DMA to send data

    *(volatile unsigned*)PCM_CS_A = 1;  //EN PCM interface
    *(volatile unsigned*)PCM_CS_A |= (1 << 25); //disable StAndyBy ram

    pcm_init_tx();
    pcm_init_modea();

    *(volatile unsigned*)PCM_CS_A |= (1 << 3); //Assert to clear TX FIFO
    *(volatile unsigned*)PCM_CS_A |= (1 << 24);
    while(!(*(volatile unsigned*)PCM_CS_A & (1 << 24))); //wait 2 PCM clock

    *(volatile unsigned*)PCM_CS_A |= (1 << 9); //enable DMA REQ
    

    //*(volatile unsigned*)PCM_CS_A &= (~(0b11 << 5)); //set TX FIFO threshold for TXW when fifo is half //useless with DMA
    //*(volatile unsigned*)PCM_CS_A |= (0b10 << 5);

    //pcm_fill_fifo();
}

void pcm_start_transmission() {
    *(volatile unsigned*)PCM_CS_A |= (1 << 2);  //enable transmission
}

/*
void send_data_to_pcm() {
    static unsigned int state_freq = 0;
    const unsigned int do_incr = 42852281; // 440 * 2^(32) / 44100

    if (*(volatile unsigned*)PCM_CS_A & (1 << 17)) {
        for (unsigned char i = 0; i < 32; i+2) {
            if (state_freq >= 4294967296)
                state_freq -= 4294967296;
            
            float s = sinf(state_freq);
            
            short value = (short)(s * 32767.0f);
            
            *(volatile unsigned*)PCM_FIFO_A = ((unsigned short)value << 16) | (unsigned short)value;
            *(volatile unsigned*)PCM_FIFO_A = ((unsigned short)value << 16) | (unsigned short)value;
            
            state_freq += do_incr;
        }
    }
}*/
