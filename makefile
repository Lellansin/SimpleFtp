
CC = gcc -g
OBJ= io.o server.o com.o list.o config.o public.o net.o hash.o sem.o speed.o daemon.o

hdhftpd:${OBJ}
	${CC}  $^ -o $@ -lm -lcrypt
.c.o:
	${CC}  -c $<

clear :
	rm *.o *~ -rf
