#include <xc.h>
#include <stdint.h>

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

#define LCD_RS PORTBbits.RB5
#define LCD_EN PORTBbits.RB6
#define LCD_D4 PORTBbits.RB1
#define LCD_D5 PORTBbits.RB2
#define LCD_D6 PORTBbits.RB3
#define LCD_D7 PORTBbits.RB4

volatile uint16_t t1_overflow = 0;

void lcd_pulse(void)
{
    LCD_EN = 1;
    __delay_us(1);
    LCD_EN = 0;
    __delay_us(100);
}

void lcd_send4(uint8_t n)
{
    LCD_D4 = (n >> 0) & 1;
    LCD_D5 = (n >> 1) & 1;
    LCD_D6 = (n >> 2) & 1;
    LCD_D7 = (n >> 3) & 1;
    lcd_pulse();
}

void lcd_cmd(uint8_t cmd)
{
    LCD_RS = 0;
    lcd_send4(cmd >> 4);
    lcd_send4(cmd & 0x0F);

    if (cmd == 0x01 || cmd == 0x02)
        __delay_ms(2);
    else
        __delay_us(50);
}

void lcd_data(uint8_t data)
{
    LCD_RS = 1;
    lcd_send4(data >> 4);
    lcd_send4(data & 0x0F);
    __delay_us(50);
}

void lcd_goto(uint8_t row, uint8_t col)
{
    if (row == 0)
        lcd_cmd(0x80 | col);
    else
        lcd_cmd(0x80 | (0x40 + col));
}

void lcd_puts(const char *s)
{
    while (*s)
        lcd_data(*s++);
}

void lcd_clear_line(uint8_t row)
{
    lcd_goto(row, 0);
    lcd_puts("                    "); //ciscenje ekrana
}

void lcd_init(void)
{
    ANSEL = 0x00;
    ANSELH = 0x00;

    TRISB = 0x00;
    PORTB = 0x00;

    __delay_ms(20);

    lcd_send4(0x03);
    __delay_ms(5);
    lcd_send4(0x03);
    __delay_us(150);
    lcd_send4(0x03);
    __delay_us(150);
    lcd_send4(0x02);
    __delay_us(150);

    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x01);
    lcd_cmd(0x06);
    __delay_ms(2);
}

void uint32_to_str(uint32_t n, char *s)
{
    char temp[11];
    uint8_t i = 0;
    uint8_t j = 0;

    if (n == 0)
    {
        s[0] = '0';
        s[1] = '\0';
        return;
    }

    while (n > 0)
    {
        temp[i++] = (n % 10) + '0';
        n /= 10;
    }

    while (i > 0)
        s[j++] = temp[--i];

    s[j] = '\0';
}

void __interrupt() isr(void)
{
    if (PIR1bits.TMR1IF)
    {
        PIR1bits.TMR1IF = 0;
        t1_overflow++;
    }
}

void freq_init(void)
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

uint32_t measure_count_1s(void)
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

    // prescaler 8/1
    count = count * 8UL;

    return count;
}

void lcd_print_freq(uint32_t f)
{
    char txt[11];

    lcd_goto(1, 0);
    lcd_puts("                    ");

    if (f < 1000)
    {
        lcd_goto(1, 6);
        uint32_to_str(f, txt);
        lcd_puts(txt);

        lcd_goto(1, 17);
        lcd_puts(" Hz");
    }
    else if (f < 1000000)
    {
        uint32_t khz = f / 1000;
        uint32_t hz  = f % 1000;

        lcd_goto(1, 6);

        uint32_to_str(khz, txt);
        lcd_puts(txt);
        lcd_data('.');

        lcd_data((hz / 100) + '0');
        lcd_data(((hz / 10) % 10) + '0');
        lcd_data((hz % 10) + '0');

        lcd_goto(1, 16);
        lcd_puts(" kHz");
    }
    else
    {
        uint32_t mhz = f / 1000000;
        uint32_t dec = (f % 1000000) / 1000;

        lcd_goto(1, 7);

        uint32_to_str(mhz, txt);
        lcd_puts(txt);
        lcd_data('.');

        lcd_data((dec / 100) + '0');
        lcd_data(((dec / 10) % 10) + '0');
        lcd_data((dec % 10) + '0');

        lcd_goto(1, 16);
        lcd_puts(" MHz");
    }
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