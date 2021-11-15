/*
 * =====================================================================
 * NAME         : Main.c
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

#define  NonPal_Fb   ((volatile unsigned short(*)[480]) FRAME_BUFFER)

struct Data_fmt {
	char data[12];
	int front;
};

struct Data_fmt data_uart;

volatile int time_color;
volatile int bg_color;

void init_data_uart(void);

// color
void init_colors(void);

// init
void Init(void)
{
	
	Uart_Init(115200);	
	init_data_uart();
	init_lcd();
	init_time();
	init_touch();
	init_colors();
	paint_init(time_color, bg_color);
	init_timer1();

}

// main
void Main(void)
{	

	Uart_Send_Byte('\n');
	Uart_Send_Byte('A');	
	Uart_Send_String("Hello LCD Test...!!!\n");

	
	// initiates
	Init();

	// char temp;
	// while (1)
	// {
	// 	Uart2_Send_Byte('O');
	// 	while ((temp = Uart2_Get_Char()) != 'O');
	// 	Uart_Printf("[][][]...%c\n", temp);
	// 	temp = 'X';
	// 	int i;
	// 	for (i=0;i<0xfffff;i++);
	// }

	while (1)
	{
		
		Uart2_Send_Byte('O');	
		while (Uart2_Get_Char() != 'O')
		{
			Uart2_Send_Byte('O');	
		}

		get_data_uart();

		Uart_Printf("[[%s]]\n", data_uart.data);
		if (data_uart.front > 0 && data_uart.data[data_uart.front-1] == 'P')
		{
			
			if (data_uart.data[0] == 'T') //temperature
			{
				// debug
				data_uart.data[data_uart.front] = 0;
				Uart_Printf("%s\n", data_uart.data);
				
				paint_temperature(1, data_uart.data, 62, 125, 0x0, 0xffff);
				paint_temperature(0, data_uart.data, 286, 125, 0x0, 0xffff);
			}

			else if (data_uart.data[0] == 'C') // current
			{
				// debug
				data_uart.data[data_uart.front] = 0;
				Uart_Printf("%s\n", data_uart.data);

				// paint current
			}

			init_data_uart();

		}

	}

	Uart_Send_String("Test Finished...!!!\n");
}	

void init_data_uart(void)
{

	int i;
	for (i = 0; i < 12; i++)
		data_uart.data[i] = 0;
	
	data_uart.front = 0;

}

/* color */
void init_colors(void)
{
	time_color = 0x0;
	bg_color = 0xffff;
}

/* get data */
void get_data_uart(void)
{

	Uart_Printf("value: ");
	int data = Uart2_Get_Char();
	Uart_Printf("%c\n", data);

	if (data == 0)
	{
		return;
	}

	else
	{
		data_uart.data[data_uart.front++] = data;
	}

}
