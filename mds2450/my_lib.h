#ifndef _MY_LIB_H_
#define _MY_LIB_H_

// Lcd.c
extern void init_lcd(void);
extern void paint_init(int time_color, int bg_color);
extern void paint_time(int hour, int min, int sec, int color, int bg_color);
// void paint_hour0(int hour0, int color, int bg_color);
// void paint_hour1(int hour1, int color, int bg_color);
// void paint_min0(int min0, int color, int bg_color);
// void paint_min1(int min1, int color, int bg_color);
extern void paint_temperature(int celsi, char* data, const int x, const int y, const int color, const int bg_color);
// extern void paint_rect(const int x, const int y, const int size_x, const int size_y, const int color);

// Uart.c 
extern void Uart_Init(int baud);
extern void Uart_Printf(char *fmt,...);
extern void Uart_Send_String(char *pt);
extern void Uart_Send_Byte(int data);
extern char Uart_Get_Char(void);
extern char Uart2_Get_Char(void);
extern void Uart2_Send_Byte(char data);

// Time.c
extern unsigned int CUR_SEC;

extern void init_time(void);
extern void set_current_time(const unsigned int hour, const unsigned int min, const unsigned int sec);
extern void get_current_time(unsigned int* hour, unsigned int* min, unsigned int* sec);
extern void inc_time(const unsigned int sec);

// Timer.c
extern void init_timer1(void);
extern void stop_timer1(void);

// Touch.c
extern void init_touch(void);

#endif
