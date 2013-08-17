
#include "net.h"

#include <sys/types.h>
#include <sys/socket.h>

// 设置地址重用
int serAddrReuse(int listenfd)
{
	//设置地址重用
	int opt = 1;
	int rel = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(opt));
	return rel;
}


