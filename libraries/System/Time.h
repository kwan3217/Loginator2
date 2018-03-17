#ifndef TIME_H
#define TIME_H

#include <inttypes.h>

#define FOSC 12000000
extern unsigned int PCLK,CCLK,timerInterval;
static const unsigned int timerSec=60;

void setup_clock(void);
void delay(unsigned int ms);
void setup_pll(unsigned int channel, unsigned int M);
void set_rtc(int y, int m, int d, int h, int n, int s);
uint32_t uptime();
#endif
