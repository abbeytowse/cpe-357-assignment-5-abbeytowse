CC = gcc
CFLAGS = -Wall -std=c99 -pedantic 
HTTPD= httpd
HTTPD_OBJS = httpd.o net.o array_list.o
PROGS = $(HTTPD) 

all : $(PROGS)

$(HTTPD) : $(HTTPD_OBJS)
	$(CC) $(CFLAGS) -o $(HTTPD) $(HTTPD_OBJS)

server.o : httpd.c net.h
	$(CC) $(CFLAGS) -c httpd.c

net.o : net.c net.h
	$(CC) $(CFLAGS) -c net.c

array_list.o : array_list.c array_list.h 
	$(CC) $(CFLAGS) -c array_list.c

clean :
	rm *.o $(PROGS) core
