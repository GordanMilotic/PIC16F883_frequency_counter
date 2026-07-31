#include "freq_counter.h"

#define _XTAL_FREQ 16000000UL

volatile uint16_t t1_overflow = 0;

void freq_init(void) //inicijalizaciranje timer1 kao impulse counter
{
    TRISCbits.TRISC0 = 1;
    ANSELbits.ANS4 = 0;

    T1CONbits.TMR1CS = 1;
    T1CONbits.T1SYNC = 1;
    T1CONbits.T1OSCEN = 0;
    T1CONbits.T1CKPS = 0b00;
    T1CONbits.TMR1ON = 0;

    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;

    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
}

void freq_timer1_overflow_handler(void) //overflow za timer1 i povecanje overflowa
{
    if (PIR1bits.TMR1IF)
    {
        PIR1bits.TMR1IF = 0;
        t1_overflow++;
    }
}

uint32_t measure_count_1s(void) //brojanje impulsa svake sekunde i vraca izmjerenu frekvenciju
{
    uint32_t count;
    uint8_t h, l;

    INTCONbits.GIE = 0;
    t1_overflow = 0;
    TMR1H = 0;
    TMR1L = 0;
    PIR1bits.TMR1IF = 0;
    INTCONbits.GIE = 1;

    T1CONbits.TMR1ON = 1;

    __delay_ms(1000);

    T1CONbits.TMR1ON = 0;

    if (PIR1bits.TMR1IF)
    {
        PIR1bits.TMR1IF = 0;
        t1_overflow++;
    }

    h = TMR1H;
    l = TMR1L;

    count = ((uint32_t)t1_overflow << 16) |
            ((uint16_t)h << 8) |
            l;

    count = count * 8UL; //prescaler opcija
    /*
     * /2, /4, /8, /16 fizicki pinovi: 3, 4, 5, 6
     */
 
    return count;
}