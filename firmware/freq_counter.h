#ifndef FREQ_COUNTER_H
#define FREQ_COUNTER_H

#include <xc.h>
#include <stdint.h>

void freq_init(void);
uint32_t measure_count_1s(void);
void freq_timer1_overflow_handler(void);

#endif