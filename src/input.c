#include "input.h"
#include "peripherals/gpio.h"
#include "utils.h"
#include <stdint.h>

/**
 * @brief Set all pin for input button
 */
void input_init() {
    *(volatile unsigned *)GPFSEL1 &= ~(0b111 << 21); // set input for GPIO17

    *(volatile unsigned *)GPAREN0 |= (1 << 17); // activate Asynchronous input for gpio17

    *(volatile unsigned *)GPPUD = 0;
    delay(150);
    *(volatile unsigned *)GPPUDCLK0 = (1 << 17);
    delay(150);
    *(volatile unsigned *)GPPUDCLK0 = 0;
}

int is_button_pressed() {
    int status = (*(volatile unsigned *)GPEDS0 & (1 << 17)) ? 1 : 0; // get value

    *(volatile unsigned *)GPEDS0 = (1 << 17); // clear bit status

    delay(200000);

    return status;
}

// GPIOB
#define SH_LD 10  // entrée
#define CLK 11    // entrée
#define SERIAL 12 // sortie

#define NB_BUTT 8

void delay_tick(volatile int d) {
    while (d--)
        ;
}

void gpio_set(int nb_pin) { put32(GPSET0, (1 << nb_pin)); }
void gpio_clear(int nb_pin) { put32(GPCLR0, (1 << nb_pin)); }

void toggle_sh() {
    gpio_clear(SH_LD);
    delay_tick(200);
    gpio_set(SH_LD);
    delay_tick(200);
}
void toggle_clk() {
    gpio_set(CLK);
    delay_tick(100);
    gpio_clear(CLK);
    delay_tick(100);
}

void init_led() {
    unsigned int s;
    s = get32(GPFSEL0);
    s &= ~(7 << (9 * 3));
    s |= 1 << (9 * 3);
    put32(GPFSEL0, s);
}

void led_on() { gpio_set(9); }
void led_off() { gpio_clear(9); }

void Init_Pin_Keyboard() {
    unsigned int selector;

    selector = get32(GPFSEL1);
    selector &= ~(7 << 0); // clean gpio10
    selector |= 1 << 0;    // put gpio10 as output
    selector &= ~(7 << 3); // clean gpio11
    selector |= 1 << 3;    // put gpio11 as output
    selector &= ~(7 << 6); // clean gpio12
    selector |= 0 << 6;    // put gpio12 as input
    put32(GPFSEL1, selector);

    put32(GPPUD, 1);
    delay(150);
    put32(GPPUDCLK0, (1 << SERIAL)); // put pin 12 to Pull-Down
    delay(150);
    put32(GPPUD, 0);
    put32(GPPUDCLK0, 0);
}

uint64_t normalize_keyboard(uint64_t raw) {
    uint64_t v = 0;
    for (int j = 0; j < 5; j++) {

        for (int i = 0; i < 8; i++) {
            int raw_i = j * 8 + i;
            int norm_i = j * 8 + (7 - i);

            if (raw & (1ULL << raw_i))
                v |= (1ULL << norm_i);
        }
    }
    return v;
}

uint64_t reading_inputs(void) {
    uint64_t v = 0;

    toggle_sh();
    for (int i = 0; i < 40; i++) {
        uint32_t value = (get32(GPLEV0) & (1 << SERIAL));
        v |= ((value ? 1ULL : 0ULL) << i);

        toggle_clk();
    }

    uint64_t v1 = normalize_keyboard(v);
    return v1;
}
