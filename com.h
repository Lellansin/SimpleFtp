
#ifndef _COM_H_
#define _COM_H_

#define COUNT_DOWN_SIG 50
#define COUNT_UP_SIG 51

#define PATH_LEN 250
#define BUFSIZE 1024

typedef struct _session
{
	char com[PATH_LEN];
	char param[PATH_LEN];
	char dir[PATH_LEN];
} SESSION ;

void auto_switch(SESSION *nowsession);
void get_com  (char *recvbuf, SESSION *nowsession);
void get_param(char *recvbuf, SESSION *nowsession);
void get_dir  (char *recvbuf, SESSION *nowsession);

void do_user(SESSION *nowsession);
void do_pass(SESSION *nowsession);
void do_syst(SESSION *nowsession);
void do_feat(SESSION *nowsession);
void do_clnt(SESSION *nowsession);
void do_pwd (SESSION *nowsession);
void do_type(SESSION *nowsession);
void do_rest(SESSION *nowsession);
void do_pasv(SESSION *nowsession);
void do_port(SESSION *nowsession);
void do_list(SESSION *nowsession);
void do_cwd (SESSION *nowsession);
void do_mkd (SESSION *nowsession);
void do_rmd (SESSION *nowsession);
void do_dele(SESSION *nowsession);
void do_rnfr(SESSION *nowsession);
void do_rnto(SESSION *nowsession);
void do_site(SESSION *nowsession);
void do_cdup(SESSION *nowsession);
void do_stor(SESSION *nowsession);
void do_retr(SESSION *nowsession);
void do_size(SESSION *nowsession);
void do_mdtm(SESSION *nowsession);

void statistics();

#endif
