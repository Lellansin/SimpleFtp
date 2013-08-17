
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

void update_clock();

int count;

void move_to(int x, int y)
{
	printf("\033[%d;%dH\n", x, y);
}

void speed_limit()
{
	struct itimerval tick;     
	// 初始化结构体     
	memset(&tick, 0, sizeof(tick));     
	// 第一次执行函数的延迟时间     
	tick.it_value.tv_sec = 1;  // sec     
	tick.it_value.tv_usec = 0; // micro sec.     
	// 执行函数的间隔时间
	tick.it_interval.tv_sec = 1;     
	tick.it_interval.tv_usec = 0;     
	// Set timer, ITIMER_REAL : real-time to decrease timer,     
	//            send SIGALRM when timeout     
	int res = setitimer(ITIMER_REAL, &tick, NULL);     
	if (res) {     
		printf("Set timer failed!!\n");     
	}

	printf("成功设置信号发送。\n");
}

void init_clock()
{
	signal(SIGALRM, update_clock);
	// update_clock();
}

void update_clock(int sig)
{
	static int i = 0;
	char time_str[10] = {0, };
	time_t t = time(NULL);
	struct tm *cur_time;

	cur_time = localtime(&t);
	
	sprintf(time_str, "%02d:%02d:%02d",
		cur_time->tm_hour,
		cur_time->tm_min,
		cur_time->tm_sec);

	// move_to(2, 10);
	printf("第%d次收到信号 %d ", count++, sig);
	printf("%s\n", time_str);
	fflush(stdout);
}

int main()
{
	system("clear");
	init_clock();
	speed_limit();
	
	while(1) ;
}
