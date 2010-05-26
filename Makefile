# Makefile for plantest project
#
#

DEBUG  = y
#CC	= /usr/local/bin/arm-linux-gcc
CC	= gcc

ifeq ($(DEBUG),y)
	CFLAGS += -O -g -DPT_DEBUG -Wall -Wstrict-prototypes 
else
	CFLAGS += -O2 -s
endif


all: plantest server

PHONY: clean

plantest: plantest.o pto.o syslog.o netlog.o netcmd.o lib.o

server: server.o

clean: 
	rm -f plantest server *.o *~
