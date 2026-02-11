#include "peripherals/gpio.h"
#include "utils.h"

/**
 * @brief Set all pin for input button
 */
void input_init() {
    *(volatile unsigned*)GPFSEL1 &= ~(0b111 << 21); //set input for GPIO17

    *(volatile unsigned*)GPAREN0 |= (1 << 17); //activate Asynchronous input for gpio17

    *(volatile unsigned*)GPPUD = 0;
    delay(150);
    *(volatile unsigned*)GPPUDCLK0 = (1 << 17);
    delay(150);
    *(volatile unsigned*)GPPUDCLK0 = 0;
}

int is_button_pressed() {
    int status = (*(volatile unsigned*)GPEDS0 & (1 << 17)) ? 1 : 0; //get value

    *(volatile unsigned*)GPEDS0 = (1 << 17); //clear bit status

    delay(200000);

    return status;
}
