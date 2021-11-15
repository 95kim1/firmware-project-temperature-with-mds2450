/*
 * =====================================================================
 * NAME         : Lcd.c
 *
 * Descriptions : Main routine for S3C2450
 *
 * IDE          : GCC-4.1.0
 *
 * Modification
 *	  
 * =====================================================================
 */
#include "2450addr.h"
#include "my_lib.h"
#include "option.h"
#include "./Images/Numbers/num0.h"
#include "./Images/Numbers/num1.h"
#include "./Images/Numbers/num2.h"
#include "./Images/Numbers/num3.h"
#include "./Images/Numbers/num4.h"
#include "./Images/Numbers/num5.h"
#include "./Images/Numbers/num6.h"
#include "./Images/Numbers/num7.h"
#include "./Images/Numbers/num8.h"
#include "./Images/Numbers/num9.h"
#include "./Images/Numbers/num_dots.h"
#include "./Images/Numbers/num_dot.h"
#include "./Images/frame.h"
#include "./Images/celsius.h"
#include "./Images/farenheit.h"

#define LCD_SIZE_X (480)
#define LCD_SIZE_Y (272)
#define CLKVAL  	13
#define HOZVAL 	   (LCD_SIZE_X-1)
#define LINEVAL    (LCD_SIZE_Y-1)

#define VBPD (10)	
#define VFPD (1)	
#define VSPW (1)	
#define HBPD (43)	
#define HFPD (1)	
#define HSPW (1)	

#define  NonPal_Fb   ((volatile unsigned short(*)[480]) FRAME_BUFFER)
#define  Fb   ((volatile unsigned long(*)[80]) FRAME_BUFFER)
//unsigned short int (* NonPal_Fb)[480]; 
//unsigned long (* Fb)[80];

//Function Declaration
void Lcd_Port_Init(void);
void NonPal_Lcd_Init(void);
void NonPal_Put_Pixel(int x, int y, int color);
void Lcd_Draw_BMP(int x, int y, const unsigned char *fp);

static unsigned short bfType;
static unsigned int bfSize;
static unsigned int bfOffbits;
static unsigned int biWidth, biWidth2;
static unsigned int biHeight;

/* init */
void init_lcd(void);
void paint_init(int time_color, int bg_color);

/* get buffer */
#define NUM_SIZE_X 18
#define NUM_SIZE_Y 30

#define DOT_SIZE_X 6
#define DOT_SIZE_Y 6

#define FRAME_SIZE_X 180
#define FRAME_SIZE_Y 140

#define SYMBOL_TEMP_SIZE_X 28
#define SYMBOL_TEMP_SIZE_Y 25

unsigned int num_buffer[10][NUM_SIZE_Y][NUM_SIZE_X];

const unsigned char* num_hex[10] = {num0, num1, num2, num3, num4, num5, num6, num7, num8, num9};

unsigned int dot_buffer[DOT_SIZE_Y][DOT_SIZE_X];

unsigned int celsius_buffer[SYMBOL_TEMP_SIZE_Y][SYMBOL_TEMP_SIZE_X];
unsigned int farenheit_buffer[SYMBOL_TEMP_SIZE_Y][SYMBOL_TEMP_SIZE_X];
unsigned int frame_buffer[FRAME_SIZE_Y][FRAME_SIZE_X];

static void get_frame_buffer(void);
static void get_number_buffer(int num);
static void get_dot_buffer(void);
static void get_symbol_temp_buffer(unsigned int (*)[SYMBOL_TEMP_SIZE_X], const unsigned char*);


/* number */
void paint_number(const int x, const int y, const int num, const int color, const int bg_color);
void paint_dot(const int x, const int y, const int color);

/* Time */
#define HOUR0_X 191
#define HOUR0_Y 15

#define HOUR1_X 214
#define HOUR1_Y 15

#define MIN0_X  248
#define MIN0_Y  15

#define MIN1_X  271
#define MIN1_Y  15

#define SEC0_X  305
#define SEC0_Y  15

#define SEC1_X  328
#define SEC1_Y  15

#define DOT0_X  237
#define DOT0_Y  21

#define DOT1_X  237
#define DOT1_Y  33

void paint_time(int hour, int min, int sec, int color, int bg_color);
void paint_hour0(int hour0, int color, int bg_color);
void paint_hour1(int hour1, int color, int bg_color);
void paint_min0(int min0, int color, int bg_color);
void paint_min1(int min1, int color, int bg_color);
void paint_sec0(int sec0, int color, int bg_color);
void paint_sec1(int sec1, int color, int bg_color);


/* Background */
void paint_bg(const int color);

/* FRAME */
#define TEMP_FRAME_X 38
#define TEMP_FRAME_Y 75

#define CURRENT_FRAME_X 262
#define CURRENT_FRAME_Y 75

void paint_frame(const int y, const int x, const int color);

/* temperature */
#define SYMBOL_TEMP_X 110
#define SYMBOL_TEMP_Y 170

void paint_temperature(int celsi, char* data, const int x, const int y, const int color, const int bg_color);

void paint_temp_symbol(unsigned int (*temp_symbol_buffer)[SYMBOL_TEMP_SIZE_X], const int x, const int y, const int color);

/* current */
#define SYMBOL_CURR_X 334
#define SYMBOL_CURR_Y 170


//
void paint_rect(const int x, const int size_x, const int y, const int size_y, const int color);

///////////

void Lcd_Port_Init(void)
{
	
	// pull-up/down not available
	rGPCUDP = 0xffffffff;
	// configuration: GPC[15:8]:=RGB_VD[7:0], GPC[4:0]:=RGB_[VDEN,VSYNC,HSYNC,VCLK,LEND];
	rGPCCON = ((rGPCCON & ~0xffff03ff) | 0xaaaa02aa); // conserve GPC5,6,7 values
	
	// pull-up/down not available
	rGPDUDP = 0xffffffff; 
	// configuration: GPD[15:0]:=RGB_VD[23:8]
	rGPDCON = ((rGPDCON & ~0xffffffff) | 0xaaaaaaaa);
	
	// configruation: GPG12:=LCD_enable
	rGPGCON=(rGPGCON&~(3<<24))|(1<<24); //GPG12:=
	// ON(enable)
	rGPGDAT|=(1<<12);
	
	// configuration: GPB0:=LCD backlight
	rGPBCON = (rGPBCON & ~(3<<0)) | (1<<1);
	// ON(enable)
	rGPBDAT|= (1<<0);
	
	
}

void NonPal_Lcd_Init()
{
	/* video window 0's buffer 0 */
	rVIDW00ADD0B0 = FRAME_BUFFER; 											// buffer start address register
	// FRAME_BUFFER = 0x33800000, 0x33000000: bank location, 0x800000: start address
	rVIDW00ADD1B0 = 0; 																	// buffer end address register
	rVIDW00ADD2B0 = (0<<13)|((LCD_SIZE_X*4*2)&0x1fff);	// buffer size register

	rVIDW00ADD1B0 = 0+(LCD_SIZE_X*LCD_SIZE_Y);
	//
	
	/* TO DO : setting for LCD control 
	* RGB I/F,PNRMODE(BGR),VCLK=9MHz,VCLK=Enable,CLKSEL=HCLK,CLKDIR=Divided,ENVID=disable
	*/
	rVIDCON0=(0x0<<22)+(0x1<<13)+((CLKVAL)<<6)+(1<<5)+(1<<4)+(0<<2);
	rVIDCON1= (1<<6)+(1<<5);
	
	rVIDTCON0=((VBPD)<<16)+((VFPD)<<8)+(VSPW);
	rVIDTCON1=((HBPD)<<16)+((HFPD)<<8)+(HSPW);
	rVIDTCON2=(LINEVAL<<11)+(HOZVAL);
	
	rVIDOSD0A		= 	(((0)&0x7FF)<<11) | (((0)&0x7FF)<<0);
	rVIDOSD0B	 	= 	(((LCD_SIZE_X-1)&0x7FF)<<11) | (((LCD_SIZE_Y-1)&0x7FF)<<0);

	rVIDOSD1A 		= 	(((0)&0x7FF)<<11) | (((0)&0x7FF)<<0);
	rVIDOSD1B	 	= 	(((LCD_SIZE_X-1)&0x7FF)<<11) | (((LCD_SIZE_Y-1)&0x7FF)<<0);
	/* TO DO 
	* Half swap Enable, 5:5:5:I format, Window0 ON
	*/
	rWINCON0=(0x1<<16)+(0x7<<2)+(1<<0); 
	
	/* TO DO : ENVID Enable for video output and LCD control signal */
	rVIDCON0 |= (3<<0);	
}

void NonPal_Put_Pixel(int x, int y, int color)
{
	/* TO DO : Fill FRAMEBUFFER with color value  */
	NonPal_Fb[y][x] = (unsigned short int)color;	
	
}

void Lcd_Draw_BMP(int x, int y, const unsigned char *fp)
{
     int xx=0, yy=0;	
     unsigned int tmp;
     unsigned char tmpR, tmpG, tmpB;
	
     bfType=*(unsigned short *)(fp+0);
     bfSize=*(unsigned short *)(fp+2);
     tmp=*(unsigned short *)(fp+4);
     bfSize=(tmp<<16)+bfSize;
     bfOffbits=*(unsigned short *)(fp+10);
     biWidth=*(unsigned short *)(fp+18);
     biHeight=*(unsigned short *)(fp+22);    
     biWidth2=(bfSize-bfOffbits)/biHeight;	
     for(yy=0;yy<biHeight;yy++)
     {
         for(xx=0;xx<biWidth;xx++)
         {
             tmpB=*(unsigned char *)(fp+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+0);
             tmpG=*(unsigned char *)(fp+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+1);
             tmpR=*(unsigned char *)(fp+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+2);
             tmpR>>=3;
             tmpG>>=3;
             tmpB>>=3;
             
             if(xx<biWidth2) NonPal_Put_Pixel(x+xx,y+yy,(tmpR<<10)+(tmpG<<5)+(tmpB<<0));
         } 
     }
    
}

/* init */
void init_lcd(void)
{
	int i;

	// buffer
	get_frame_buffer();
	for (i=0;i<10;i++)
		get_number_buffer(i);
	get_dot_buffer();
	get_symbol_temp_buffer(celsius_buffer, celsius);
	get_symbol_temp_buffer(farenheit_buffer, farenheit);
	
	// init (configuration)
	Lcd_Port_Init();
	NonPal_Lcd_Init();
}

void paint_init(int time_color, int bg_color)
{

	paint_bg(bg_color);
	paint_time(0, 0, 0, time_color, bg_color);
	paint_frame(TEMP_FRAME_X, TEMP_FRAME_Y, 0);
	paint_frame(CURRENT_FRAME_X, CURRENT_FRAME_Y, 0);

	paint_dot(125, 148, 0);
	paint_dot(349, 148, 0);

	paint_temp_symbol(celsius_buffer, SYMBOL_TEMP_X, SYMBOL_TEMP_Y, 0);
	paint_temp_symbol(farenheit_buffer, SYMBOL_CURR_X, SYMBOL_CURR_Y, 0);
}

/* get buffer */
static void get_number_buffer(int num)
{
	int xx=0, yy=0;	
	unsigned int tmp;
	unsigned char tmpR, tmpG, tmpB;

	bfType=*(unsigned short *)(num_hex[num]+0);
	bfSize=*(unsigned short *)(num_hex[num]+2);
	tmp=*(unsigned short *)(num_hex[num]+4);
	bfSize=(tmp<<16)+bfSize;
	bfOffbits=*(unsigned short *)(num_hex[num]+10);
	biWidth=*(unsigned short *)(num_hex[num]+18);    
	biHeight=*(unsigned short *)(num_hex[num]+22);    
	biWidth2=(bfSize-bfOffbits)/biHeight;	
	for(yy=0;yy<biHeight;yy++)
	{
		for(xx=0;xx<biWidth;xx++)
		{
			tmpB=*(unsigned char *)(num_hex[num]+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+0);
			tmpG=*(unsigned char *)(num_hex[num]+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+1);
			tmpR=*(unsigned char *)(num_hex[num]+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+2);
			tmpR>>=3;
			tmpG>>=3;
			tmpB>>=3;

			if(xx<biWidth2 && xx < NUM_SIZE_X)
			{
				num_buffer[num][yy][xx] = (tmpR<<10)+(tmpG<<5)+(tmpB<<0);
			}
		} 
	}
}

static void get_dot_buffer(void)
{
	int xx=0, yy=0;	
	unsigned int tmp;
	unsigned char tmpR, tmpG, tmpB;

	bfType=*(unsigned short *)(num_dot+0);
	bfSize=*(unsigned short *)(num_dot+2);
	tmp=*(unsigned short *)(num_dot+4);
	bfSize=(tmp<<16)+bfSize;
	bfOffbits=*(unsigned short *)(num_dot+10);
	biWidth=*(unsigned short *)(num_dot+18); 
	biHeight=*(unsigned short *)(num_dot+22);    
	biWidth2=(bfSize-bfOffbits)/biHeight;	
	for(yy=0;yy<biHeight;yy++)
	{
		for(xx=0;xx<biWidth;xx++)
		{
			tmpB=*(unsigned char *)(num_dot+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+0);
			tmpG=*(unsigned char *)(num_dot+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+1);
			tmpR=*(unsigned char *)(num_dot+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+2);
			tmpR>>=3;
			tmpG>>=3;
			tmpB>>=3;

			if(xx<biWidth2)
			{
				dot_buffer[yy][xx] = (tmpR<<10)+(tmpG<<5)+(tmpB<<0);
			}
		} 
	}
}

static void get_symbol_temp_buffer(unsigned int (*symbol_temp_buffer)[SYMBOL_TEMP_SIZE_X], const unsigned char* symbol_temp)
{
	int xx=0, yy=0;	
	unsigned int tmp;
	unsigned char tmpR, tmpG, tmpB;

	bfType=*(unsigned short *)(symbol_temp+0);
	bfSize=*(unsigned short *)(symbol_temp+2);
	tmp=*(unsigned short *)(symbol_temp+4);
	bfSize=(tmp<<16)+bfSize;
	bfOffbits=*(unsigned short *)(symbol_temp+10);
	biWidth=*(unsigned short *)(symbol_temp+18); 
	biHeight=*(unsigned short *)(symbol_temp+22);    
	biWidth2=(bfSize-bfOffbits)/biHeight;	
	for(yy=0;yy<biHeight;yy++)
	{
		for(xx=0;xx<biWidth;xx++)
		{
			tmpB=*(unsigned char *)(symbol_temp+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+0);
			tmpG=*(unsigned char *)(symbol_temp+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+1);
			tmpR=*(unsigned char *)(symbol_temp+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+2);
			tmpR>>=3;
			tmpG>>=3;
			tmpB>>=3;

			if(xx<biWidth2)
			{
				symbol_temp_buffer[yy][xx] = (tmpR<<10)+(tmpG<<5)+(tmpB<<0);
			}
		} 
	}
}

/* number */
void paint_number(const int x, const int y, const int num, const int color, const int bg_color)
{
	int yy, xx;
	for (yy = 0; yy < NUM_SIZE_Y; yy++)
	{
		for (xx = 0; xx < NUM_SIZE_X; xx++)
		{
			if (num_buffer[num][yy][xx] == 0)
				NonPal_Put_Pixel(x+xx,y+yy,color);
			else
				NonPal_Put_Pixel(x+xx,y+yy,bg_color);
		}
	}
}
void paint_dot(const int x, const int y, const int color)
{
	int yy, xx;
	for (yy = 0; yy < DOT_SIZE_Y; yy++)
	{
		for (xx = 0; xx < DOT_SIZE_X; xx++)
		{
			NonPal_Put_Pixel(x+xx,y+yy,color);
		}
	}
}

/* Time */
void paint_time(int hour, int min, int sec, int color, int bg_color)
{
	paint_hour0(hour/10, color, bg_color);
	paint_hour1(hour%10, color, bg_color);
	paint_min0(min/10, color, bg_color);
	paint_min1(min%10, color, bg_color);
	paint_sec0(sec/10, 0xabde, bg_color);
	paint_sec1(sec%10, 0xabde, bg_color);
	paint_dot(DOT0_X, DOT0_Y, color);
	paint_dot(DOT1_X, DOT1_Y, color);
}

void paint_hour0(int hour0, int color, int bg_color)
{
	paint_number(HOUR0_X, HOUR0_Y, hour0, color, bg_color);
}
void paint_hour1(int hour1, int color, int bg_color)
{
	paint_number(HOUR1_X, HOUR1_Y, hour1, color, bg_color);
}

void paint_min0(int min0, int color, int bg_color)
{
	paint_number(MIN0_X, MIN0_Y, min0, color, bg_color);
}
void paint_min1(int min1, int color, int bg_color)
{
	paint_number(MIN1_X, MIN1_Y, min1, color, bg_color);
}

void paint_sec0(int sec0, int color, int bg_color)
{
	paint_number(SEC0_X, SEC0_Y, sec0, color, bg_color);
}
void paint_sec1(int sec1, int color, int bg_color)
{
	paint_number(SEC1_X, SEC1_Y, sec1, color, bg_color);
}


/* Background */
void paint_bg(const int color)
{
	int y, x;
	for (y = 0; y < LCD_SIZE_Y; y++)
		for (x = 0; x < LCD_SIZE_X; x++)
			NonPal_Put_Pixel(x,y,color);
}

/* FRAME */
static void get_frame_buffer(void)
{
	int xx=0, yy=0;	
	unsigned int tmp;
	unsigned char tmpR, tmpG, tmpB;

	bfType=*(unsigned short *)(frame+0);
	bfSize=*(unsigned short *)(frame+2);
	tmp=*(unsigned short *)(frame+4);
	bfSize=(tmp<<16)+bfSize;
	bfOffbits=*(unsigned short *)(frame+10);
	biWidth=*(unsigned short *)(frame+18);    
	biHeight=*(unsigned short *)(frame+22);    
	biWidth2=(bfSize-bfOffbits)/biHeight;	
	for(yy=0;yy<biHeight;yy++)
	{
		for(xx=0;xx<biWidth;xx++)
		{
			tmpB=*(unsigned char *)(frame+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+0);
			tmpG=*(unsigned char *)(frame+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+1);
			tmpR=*(unsigned char *)(frame+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+2);
			tmpR>>=3;
			tmpG>>=3;
			tmpB>>=3;

			if(xx<biWidth2)
			{
				frame_buffer[yy][xx] = (tmpR<<10)+(tmpG<<5)+(tmpB<<0);
			}
		} 
	}
}

void paint_frame(const int x, const int y, const int color)
{
	int yy, xx;
	for (yy = 0; yy < FRAME_SIZE_Y; yy++)
	{
		for (xx = 0; xx < FRAME_SIZE_X; xx++)
		{
			if (frame_buffer[yy][xx] == 0)
				NonPal_Put_Pixel(x+xx, y+yy, color);
		}
	}
}

// paint temperature

static float stof(char* data, int len)
{

	char* temp = data;
	char* end = data+len;

	int d_num = 0;
	int f_num = 0;

	if (*data == '-')
		temp++;

	while (temp < end && *temp != '.')
	{
		d_num *= 10;
		d_num += *temp++ - '0';
	}

	temp++;

	int div = 1;
	while (temp < end)
	{
		div *= 10;
		f_num *= 10;
		f_num += *temp++ - '0';
	}

	if (*data == '-')
		return -(d_num + (float)f_num / div);
	return d_num + (float)f_num / div;
}

static int ftos(float f, char* str)
{

	char* temp = str;
	
	char* start, * end;

	if (f < 0) {
		*temp++ = '-';
	}

	start = temp;

	int d_num = (int)f;
	float f_num = f - d_num;

	if (d_num == 0)
		*temp++ = '0';

	while (d_num)
	{
		*temp++ = d_num % 10 + '0';
		d_num /= 10;
	}

	end = temp - 1;

	while (start < end)
	{
		char t = *start;
		*start++ = *end;
		*end-- = t;
	}

	*temp++ = '.';

	int cnt = 0;
	while (f_num != (int)f_num && cnt < 3)
	{
		cnt++;
		f_num *= 10;
		*temp++ = (int)f_num % 10 + '0';
	}

	return temp - str;
}

extern void Uart_Printf(char *fmt,...);
void paint_temperature(int celsi, char* data, const int x, const int y, const int color, const int bg_color)
{
	int i, m_flag = bg_color;
	char* temp = data + 1;

	int pos_int[3][2] = {{0,0}, {21, 0}, {42, 0}};
	int pos_float[3][2] = {{72, 0},{93, 0},{114, 0}};

	// int pos_int[3][2] = {{62,125}, {83, 125}, {104, 125}};
	// int pos_float[3][2] = {{134, 125},{155, 125},{176, 125}};

	if (celsi == 0)
	{
		char* temp = data+1;
		int len = 0;
		while (*temp++ != 'P') len++;
		float farenheit = stof(data+1, len);
		// Uart_Printf("before: %d", (int)farenheit);
		// Uart_Printf(".%d\n", (int)((farenheit-(int)farenheit)*1000));
		farenheit = 9*farenheit/5 + 32;
		farenheit = (int)farenheit + (int)((farenheit-(int)farenheit)*1000)/1000.0;
		
		data[0] = 'C';
		data[ftos(farenheit, data+1)+1] = 'P';
		// data[11] = 0;
		// Uart_Printf("after: %s\n", data);
		// return;
	}

	if (*temp == '-')
	{
		temp++;
		m_flag = color;
	}

	for (i = 0; i < 3 && temp[i] != '.' && temp[i] != 'P'; i++);
	i = 3 - i;
	
	// paint '-'
	{
		int yy, xx;
		for (yy=0;yy<3;yy++)
		{
			for(xx=0;xx<10;xx++)
			{
				NonPal_Put_Pixel(x+xx+pos_int[i][0]-15, y+yy+pos_int[i][1]+14, m_flag);
			}
		}
	}
	{
		int j;
		for (j = 0; j < i; j++)
			paint_rect(x+pos_int[j][0], NUM_SIZE_X, y+pos_int[j][1], NUM_SIZE_Y, bg_color);
	}

	// paint integer
	while (*temp != '.' && *temp != 'P')
	{
		// paint
		paint_number(x+pos_int[i][0], y+pos_int[i][1], *temp-'0', color, bg_color);
		i++;
		temp++;
	}	

	temp++;

	// paint others (except '.')
	i = 0;
	while (*temp != '0' && *temp != 'P')
	{
		// paint
		paint_number(x+pos_float[i][0], y+pos_float[i][1], *temp-'0', color, bg_color);
		i++;
		temp++;
	}

	if (i == 0)
	{
		paint_number(x+pos_float[i][0], y+pos_float[i][1], 0, color, bg_color);
	}

}

void paint_temp_symbol(unsigned int (*temp_symbol_buffer)[SYMBOL_TEMP_SIZE_X], const int x, const int y, const int color)
{
	int yy, xx;
	for (yy = 0; yy < SYMBOL_TEMP_SIZE_Y; yy++)
	{
		for (xx = 0; xx < SYMBOL_TEMP_SIZE_X; xx++)
		{
			if (temp_symbol_buffer[yy][xx] == 0)
				NonPal_Put_Pixel(x+xx, y+yy, color);
		}
	}
}

// paint current value
void paint_current(char* str)
{


}

void paint_rect(const int x, const int size_x, const int y, const int size_y, const int color)
{
	int yy, xx;
	for (yy = 0; yy < size_y; yy++)
	{
		for (xx = 0; xx < size_x; xx++)
		{
			NonPal_Put_Pixel(x+xx, y+yy, color);
		}
	}
}

