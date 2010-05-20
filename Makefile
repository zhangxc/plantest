# Makefile for plantest project
#
#

DEBUG  = y
#CC	= /usr/local/bin/arm-linux-gcc
CC	= gcc
CFLAGS  += -Wall -Wstrict-prototypes 

ifeq ($(DEBUG),y)
	CFLAGS += -O -g -DPT_DEBUG
else
	CFLAGS += -O2 -s
endif


all: plantest

PHONY: clean

plantest: plantest.o pto.o syslog.o netlog.o lib.o

clean: 
	rm -f plantest *.o *~
