#ifndef LCD_H
#define LCD_H

#include <xc.h>
#include <stdint.h>

#define LCD_RS PORTBbits.RB5
#define LCD_EN PORTBbits.RB6
#define LCD_D4 PORTBbits.RB1
#define LCD_D5 PORTBbits.RB2
#define LCD_D6 PORTBbits.RB3
#define LCD_D7 PORTBbits.RB4

void lcd_init(void);
void lcd_cmd(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_goto(uint8_t row, uint8_t col);
void lcd_puts(const char *s);
void lcd_clear_line(uint8_t row);
void lcd_print_freq(uint32_t f);

#endif