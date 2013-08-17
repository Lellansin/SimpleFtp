#include <netdb.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int get_gw_ip(char *eth, char *ipaddr)
{
 int sock_fd;
 struct  sockaddr_in my_addr;
 struct ifreq ifr;

 /**//* Get socket file descriptor */
 if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
 {
  perror("socket");
  return RET_FAILED;
 }

 /**//* Get IP Address */
 strncpy(ifr.ifr_name, eth, IF_NAMESIZE);
 ifr.ifr_name[IFNAMSIZ-1]='/0';

 if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
 {
  DBGPRN(":No Such Device %s/n",eth);
  return RET_FAILED;
 }

 memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
 strcpy(ipaddr, inet_ntoa(my_addr.sin_addr));
 close(sock_fd);
 return RET_SUCCESS;
}