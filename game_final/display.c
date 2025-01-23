/**
 * This file was created 2022 by Olle Jernström and Marcus Bardvall
 * The code was written 2015 by F Lundevall with some parts written by Axel Isaksson
 * For copyright and licensing, see file COPYING
 */
#include <stdint.h>
#include <pic32mx.h>
#include "display.h"

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)
#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

/* sleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void sleep(int cyc)
{
    int i;
    for (i = cyc; i > 0; i--)
        ;
}

void display_init(void)
{
    DISPLAY_CHANGE_TO_COMMAND_MODE;
    sleep(10);
    DISPLAY_ACTIVATE_VDD;
    sleep(1000000);

    spi_send_recv(0xAE);
    DISPLAY_ACTIVATE_RESET;
    sleep(10);
    DISPLAY_DO_NOT_RESET;
    sleep(10);

    spi_send_recv(0x8D);
    spi_send_recv(0x14);

    spi_send_recv(0xD9);
    spi_send_recv(0xF1);

    DISPLAY_ACTIVATE_VBAT;
    sleep(10000000);

    spi_send_recv(0xA1);
    spi_send_recv(0xC8);

    spi_send_recv(0xDA);
    spi_send_recv(0x20);

    spi_send_recv(0xAF);
}

uint8_t spi_send_recv(uint8_t data)
{
    while (!(SPI2STAT & 0x08))
        ;
    SPI2BUF = data;
    while (!(SPI2STAT & 1))
        ;
    return SPI2BUF;
}
