#include "2450addr.h"
#include "option.h"

#define TIMEBOX_X 191
#define TIMEBOX_Y 15
#define TIMEBOX_SIZE_X 98
#define TIMEBOX_SIZE_Y 33

#define SETBOX_X 10
#define SETBOX_Y 10
#define SETBOX_SIZE_X 98
#define SETBOX_SIZE_Y 33

#define BTN_X 114
#define BTN_Y 10
#define BTN_SIZE_X 20
#define BTN_SIZE_Y 30

#define BG_COLOR 0xffff
#define SET_COLOR 0x000e

volatile  int ADC_x, ADC_y;
volatile  int Touch_Pressed=0;

// Calibration 정보 저장 값
volatile int Cal_x1=848;
volatile int Cal_y1=656;
volatile int Cal_x2=186;
volatile int Cal_y2=349; 

volatile int setting_hour;
volatile int setting_min;
volatile int setting_on;

void init_touch(void);

static void Touch_ISR(void) __attribute__ ((interrupt ("IRQ")));
static void Touch_Init(void);

extern void Uart_Send_String(char *pt);
extern void Uart_Printf(char *fmt,...);

extern void paint_number(const int x, const int y, const int num, const int color, const int bg_color);
extern void paint_dot(const int x, const int y, const int color);
extern void NonPal_Put_Pixel(int x, int y, int color);

extern void get_current_time(unsigned int* hour, unsigned int* min, unsigned int* sec);
extern void set_current_time(const unsigned int hour, const unsigned int min, const unsigned int sec);

void init_touch(void)
{
	setting_on = 0;

	Touch_Init();

	pISR_ADC = (unsigned int)Touch_ISR;

	rINTSUBMSK &= ~(0x1<<9);
	rINTMSK1 &= ~(0x1<<31);
}

static void Touch_Init(void)
{
	rADCDLY = (50000); 
     /* TO DO : prescaler enable, prescaler value=39, Analog no input, 
      * 		Normal operation mode, Disable start, No operation */
		rADCCON |= (0x1<<14);
		rADCCON |= (39<<6);
		rADCCON &= ~(0x1<<3);
		rADCCON &= ~(0x1<<2);
		rADCCON &= ~(0x1<<1);
		rADCCON &= ~(0x1);

	 /* TO DO :  For Waiting Interrupt Mode rADCTSC=0xd3 */
		rADCTSC &= ~(0x1<<8); // detect down sig
		rADCTSC |= (0x1<<7);  // YM to GND switch enable
		rADCTSC |= (0x1<<6);  // YP to VDD switch disable
		rADCTSC &= ~(0x1<<5); // XM to GND switch disable
		rADCTSC |= (0x1<<4);  // XP to VDD switch disable
		rADCTSC &= ~(0x1<<3); // XP pull-up enable
		rADCTSC &= ~(0x1<<2);  // Auto measurement of X-Position, Y-position
		rADCTSC |= (0x1<<1);  
		rADCTSC |= (0x1);     // waiting for interrupt mode

}



static void Touch_ISR(void)
{
	/* ÀÎÅÍ·ŽÆ® Çã¿ëÇÏÁö ŸÊÀœ on Touch */
	rINTSUBMSK |= (0x1<<9);
	rINTMSK1 |= (0x1<<31);	
	
	/* TO DO: Pendng Clear on Touch */	
	rSUBSRCPND |= (0x1<<9);
	rSRCPND1 |= (0x1<<31);
	rINTPND1 |= (0x1<<31);

	if(rADCTSC & 0x100)
	{
		rADCTSC &= (0xff); 
		Touch_Pressed = 0;
		Uart_Send_String("Detect Stylus Up Interrupt Signal \n");
	}
	else
	{

		rADCTSC &=~(0x1<<8);				//detect stylus down
		rADCTSC &= ~(0x1<<7);				//set bit for x position measurement
		rADCTSC |= (0x1<<6);				//YM=Hi-z, YP=Hi-z
		rADCTSC |= (0x1<<5);		
		rADCTSC &= ~(0x1<<4);				//XM=VSS,XP=VDD
		rADCTSC &= ~(0x1<<3);				//pull-up EN
		rADCTSC &= ~(0x1<<2);				//Normal-ADC
		rADCTSC &= ~(0x3);
		rADCTSC |= (0x1);					//X-position = 1


		/* TO DO : ENABLE_START */		
		rADCCON |= (0x1);
		
		/* wait until End of A/D Conversion */
		while(!(rADCCON & (1<<15)));
		rADCCON &= ~(0x1);					//Stop Conversion

		/*Set ADCTSC reg for Y Conversion*/ 
		ADC_x = (rADCDAT0 & 0x3ff);			//Store X value
		
		rADCTSC |= (0x1<<7);				//YM=VSS, YP=VDD
		rADCTSC &= ~(0x1<<6);
		rADCTSC &= ~(0x1<<5);				//XM=Hi-z, XP=Hi-z
		rADCTSC |= (0x1<<4);
		/* clear and then set  ADCTSC [1:0] for Y Conversion*/
		rADCTSC &= ~(0x3);
		rADCTSC |= (0x2);
	
		rADCCON |= (0x1);				//StartConversion
		while(!(rADCCON & (1<<15)));	//wait untill End of A/D Conversion

		ADC_y = (rADCDAT1 & 0x3ff);		//Store y value
		
		Touch_Pressed = 1;
		
		/* TO DO : change to Waiting for interrupt mode 
		 *		   Stylus Up, YM_out Enable, YP_out Disable, XM_out Disable, XP_out disable
		 */
		rADCTSC |= (0x1<<8);
		rADCTSC |= (0x1<<7);
		rADCTSC |= (0x1<<6);
		rADCTSC &= ~(0x1<<5);
		rADCTSC |= (0x1<<4);
		rADCTSC &= ~(0x1<<3);
		rADCTSC &= ~(0x1<<2);
		rADCTSC |= (0x1<<1);
		rADCTSC |= (0x1);
			
		Uart_Printf("x: %d, y: %d\n", ADC_x, ADC_y);

		int touch_y = (ADC_y-Cal_y1)*262/(Cal_y2-Cal_y1)+5;
		int touch_x = 480-((ADC_x-Cal_x2)*470/(Cal_x1-Cal_x2)+5);
		if(touch_x<0) touch_x=0;
		if(touch_x>=480) touch_x=479;
		if(touch_y<0) touch_y=0;
		if(touch_y>=272) touch_y=271;

		Uart_Printf("x: %d, y: %d\n", touch_x, touch_y);

		if (TIMEBOX_X <= touch_x && touch_x < TIMEBOX_X + TIMEBOX_SIZE_X && TIMEBOX_Y <= touch_y && touch_y < TIMEBOX_Y + TIMEBOX_SIZE_Y)
		{
		
			setting_on = 1;

			setting_hour = 0;
			setting_min = 0;
			int temp;
			get_current_time(&setting_hour, &setting_min, &temp);

			paint_number(SETBOX_X, SETBOX_Y, setting_hour / 10, SET_COLOR, 0xffff);
			paint_number(SETBOX_X+23, SETBOX_Y, setting_hour % 10, SET_COLOR, 0xffff);
			paint_dot(SETBOX_X+46, SETBOX_Y+6, SET_COLOR);
			paint_dot(SETBOX_X+46, SETBOX_Y+18, SET_COLOR);
			paint_number(SETBOX_X+57, SETBOX_Y, setting_min / 10, SET_COLOR, 0xffff);
			paint_number(SETBOX_X+80, SETBOX_Y, setting_min % 10, SET_COLOR, 0xffff);

			int y, x;
			for (y = 0; y < BTN_SIZE_Y; y++)
			{
				for (x = 0; x < BTN_SIZE_X; x++)
				{
					NonPal_Put_Pixel(x+BTN_X, y+BTN_Y, SET_COLOR);
				}
			}	
		
		}

		else if (setting_on == 1)
		{
			
			if (SETBOX_X <= touch_x && touch_x < SETBOX_X+40 && SETBOX_Y <= touch_y && touch_y < SETBOX_Y+SETBOX_SIZE_Y)
			{
				setting_hour = (setting_hour + 1) % 24;
				paint_number(SETBOX_X, SETBOX_Y, setting_hour / 10, SET_COLOR, 0xffff);
				paint_number(SETBOX_X+23, SETBOX_Y, setting_hour % 10, SET_COLOR, 0xffff);
			}
			else if (SETBOX_X+57 <= touch_x && touch_x < SETBOX_X+SETBOX_SIZE_X && SETBOX_Y <= touch_y && touch_y < SETBOX_Y+SETBOX_SIZE_Y)
			{
				setting_min = (setting_min + 1) % 60;
				paint_number(SETBOX_X+57, SETBOX_Y, setting_min / 10, SET_COLOR, 0xffff);
				paint_number(SETBOX_X+80, SETBOX_Y, setting_min % 10, SET_COLOR, 0xffff);
			}
			else if (BTN_X <= touch_x && touch_x < BTN_X+BTN_SIZE_X && BTN_Y <= touch_y && touch_y < BTN_Y+BTN_SIZE_Y)
			{

				setting_on = 0;
				set_current_time(setting_hour, setting_min, 0);

				paint_number(SETBOX_X, SETBOX_Y, setting_hour / 10, 0xffff, 0xffff);
				paint_number(SETBOX_X+23, SETBOX_Y, setting_hour % 10, 0xffff, 0xffff);
				paint_dot(SETBOX_X+46, SETBOX_Y+6, 0xffff);
				paint_dot(SETBOX_X+46, SETBOX_Y+18, 0xffff);
				paint_number(SETBOX_X+57, SETBOX_Y, setting_min / 10, 0xffff, 0xffff);
				paint_number(SETBOX_X+80, SETBOX_Y, setting_min % 10, 0xffff, 0xffff);

				int y, x;
				for (y = 0; y < BTN_SIZE_Y; y++)
				{
					for (x = 0; x < BTN_SIZE_X; x++)
					{
						NonPal_Put_Pixel(x+BTN_X, y+BTN_Y, 0xffff);
					}
				}	

			}
		} 
	}

	/* ÀÎÅÍ·ŽÆ® ŽÙœÃ Çã¿ë  on Touch */
	rINTSUBMSK &= ~(0x1<<9);
	rINTMSK1 &= ~(0x1<<31);
	
}