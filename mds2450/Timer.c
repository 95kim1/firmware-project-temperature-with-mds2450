#include "2450addr.h"
#include "option.h"

extern int time_color;
extern int bg_color;

// timer
void init_timer1(void);
static void start_timer1(int msec);
void stop_timer1(void);

// interrupt
static void ISR_TIMER1(void) __attribute__ ((interrupt ("IRQ"))) ;

extern unsigned int CUR_SEC;

extern void init_time(void);
extern void set_current_time(const unsigned int hour, const unsigned int min, const unsigned int sec);
extern void get_current_time(unsigned int* hour, unsigned int* min, unsigned int* sec);
extern void inc_time(const unsigned int sec);

/* timer */
void init_timer1(void)
{
	rTCFG0 = 0xff;
	rTCFG1 = 3<<4;

	// timer1 auto reload on
	rTCON = 1<<11; //(0<<4)|(1<<3)|(0<<2)|(0<<1)|0;
	
	rTCNTB1 = 0;
	rTCMPB1 = 0;

	pISR_TIMER1 = (unsigned int)ISR_TIMER1;

	start_timer1(1000);
}

static void start_timer1(int msec)
{
	rTCNTB1 = 16.113*msec;
	
	// update TCNTB1, TCMPB1 to TCNT1, TCMP1
	rTCON |= (1<<9);
	rTCON &= ~(1<<9);

	rINTMSK1 &= ~(BIT_TIMER1);

	rTCON |= (1<<8);
}

void stop_timer1(void)
{
	rTCON &= ~(1<<8);
}

/* interrupt */
static void ISR_TIMER1(void)
{
	unsigned int hour, min, sec;

	// masking
	rINTMSK1 |= BIT_TIMER1;

	// pending clear
	rSRCPND1 |= BIT_TIMER1;
	rINTPND1 |= BIT_TIMER1;

	// // content1 (inc time)
	inc_time(1);
	get_current_time(&hour, &min, &sec);

	// // content2 (paint time)
	paint_time(hour, min, sec, time_color, bg_color);

	//debug
	//Uart_Printf("%d%d:%d%d %d%d\n", hour/10, hour%10, min/10, min%10, sec/10, sec%10);

	// unmasking	
	rINTMSK1 &= ~(BIT_TIMER1);
}