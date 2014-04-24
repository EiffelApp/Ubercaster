#
# Makefile for Ubercaster sender and receiver applications
#
# Author: Artem Bagautdinov, <artem.bagautdinov@gmail.com>
#	   KJ Yoo,            <kyungjindaum@gmail.com>
#

CC=gcc
STRIP=strip
CFLAGS=-std=c99 -Wall -O0 -ggdb -I/home/tim/Projects/Ubercaster/sandbox/include
LDFLAGS=-L/home/tim/Projects/Ubercaster/sandbox/lib -lopus -lasound -lpthread

.PHOHY: all clean

all: sender

sender: sender.c spsc_circular_queue.c
	$(CC) $(CFLAGS) -o sender sender.c spsc_circular_queue.c $(LDFLAGS)

clean:
	rm -rf sender
 
