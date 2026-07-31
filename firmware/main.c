#include <xc.h>
#include <stdint.h>
#include "lcd.h"
#include "freq_counter.h"

#define _XTAL_FREQ 16000000UL

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config MCLRE = ON
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF
#pragma config LVP = OFF
#pragma config BOR4V = BOR40V
#pragma config WRT = OFF

void __interrupt() isr(void)
{
    freq_timer1_overflow_handler();
}

void main(void)
{
    uint32_t count;

    lcd_init();
    freq_init();

    lcd_goto(0, 0);
    lcd_puts(" FREQUENCY  COUNTER ");

    while (1)
    {
        count = measure_count_1s();
        lcd_print_freq(count);
    }
}