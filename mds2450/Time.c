#define MAX_SEC_TIME  86400

volatile unsigned int CUR_SEC;

void init_time(void);
void set_current_time(const unsigned int hour, const unsigned int min, const unsigned int sec);
void get_current_time(unsigned int* hour, unsigned int* min, unsigned int* sec);
void inc_time(const unsigned int sec);

void init_time(void)
{
	set_current_time(0,0,0);
}

void set_current_time(const unsigned int hour, const unsigned int min, const unsigned int sec)
{
	CUR_SEC = hour*60*60 + min*60 + sec;
}

void get_current_time(unsigned int* hour, unsigned int* min, unsigned int* sec)
{
	unsigned int temp_cur_sec = CUR_SEC;
	
	*sec = temp_cur_sec % 60;
	temp_cur_sec /= 60;
	*min = temp_cur_sec % 60;
	*hour = temp_cur_sec / 60;
}

void inc_time(const unsigned int sec)
{
	CUR_SEC = (CUR_SEC + sec) % MAX_SEC_TIME;
}
