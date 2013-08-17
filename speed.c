
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <signal.h>
#include <sys/time.h>

#include "speed.h"

extern int LIMIT_SPEED;
long transfer_data_now;
long transfer_data_last;

/*
	函数功能：发送SIGALRM信号，设置为1毫秒发一次
	输入参数：char *key 配置文件中需要读取的限制速度
	返回值：无
*/
void speed_limit(int speed_conf)
{
	struct itimerval tick;     
	// 初始化结构体     
	memset(&tick, 0, sizeof(tick));     
	// 第一次执行函数的延迟时间     
	tick.it_value.tv_sec = 0;  // sec     
	tick.it_value.tv_usec = 1; // micro sec.     
	// 执行函数的间隔时间
	tick.it_interval.tv_sec = 0;     
	tick.it_interval.tv_usec = 1;     
	// Set timer, ITIMER_REAL : real-time to decrease timer,     
	//            send SIGALRM when timeout     
	int res = setitimer(ITIMER_REAL, &tick, NULL);     
	if (res) {     
		perror("setitimer");
		return;
	}
	LIMIT_SPEED = speed_conf * 1024; // 将速度转为 b/s
}

/*
	函数功能： SIGALRM 信号的处理函数，功能为限速
	输入参数：int signum: SIGALRM 信号的编号
	返回值：  无
*/

void alrm_handl(int signum)
{
	struct timespec req;
	long transfer_data = transfer_data_now - transfer_data_last; // 当前传输减去上次传输
	float speed_now;
	float temp_time;
	float mytime;
	// printf("now=[%ld], last=[%ld]\n", transfer_data_now, transfer_data_last);
	speed_now = transfer_data * 1000; // 当前的传输速度 单位转为 b/s

	if ( speed_now > LIMIT_SPEED )
	{
		memset(&req, 0, sizeof(req));
		mytime = transfer_data / LIMIT_SPEED - 0.001; // 需要睡眠的时间 单位为s
		temp_time = mytime;
		req.tv_sec = (int)temp_time; // 睡眠时间s
		req.tv_nsec = (long) ((mytime - req.tv_sec) * 1000000000 ); // 睡眠时间
		if ( req.tv_nsec < 0 )
		{
			req.tv_nsec = 0;
		}
		// printf("sec=[%ld], nsec=[%ld]\n", req.tv_sec, req.tv_nsec);
		int ret = nanosleep(&req, NULL);// 睡眠
		if (ret == -1)
		{
			perror("nanosleep");
			return;
		}
	}
	transfer_data_last = transfer_data_now;
}

/* 关闭信号 SIFALRM */
void close_timer()
{
	int ret = setitimer(ITIMER_REAL, NULL, NULL); // 1毫秒发送一次 SIGALRM 信号
	if ( ret == -1)
	{
		perror("setitimer");
		return ;
	}
	transfer_data_now = 0;
}

