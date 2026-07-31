#include "lcd.h"

#define _XTAL_FREQ 16000000UL

void lcd_pulse(void)
{
    LCD_EN = 1;
    __delay_us(1);
    LCD_EN = 0;
    __delay_us(100);
}

void lcd_send4(uint8_t n) //slanje 4 bita preko D4-D7 linija na lcd-u
{
    LCD_D4 = (n >> 0) & 1;
    LCD_D5 = (n >> 1) & 1;
    LCD_D6 = (n >> 2) & 1;
    LCD_D7 = (n >> 3) & 1;
    lcd_pulse();
}

void lcd_cmd(uint8_t cmd) //naredba lcd-u, brisanje ili pomak kursora..
{
    LCD_RS = 0;
    lcd_send4(cmd >> 4);
    lcd_send4(cmd & 0x0F);

    if (cmd == 0x01 || cmd == 0x02)
        __delay_ms(2);
    else
        __delay_us(50);
}

void lcd_data(uint8_t data) //slanje 1 znaka na lcd
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
    lcd_puts("                    ");
}

void lcd_init(void) //4 bit mode, koristimo samo pinove 11, 12, 13, 14
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