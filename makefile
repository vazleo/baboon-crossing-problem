CC = gcc
CFLAGS = -pthread -Wall

all: baboon_cross

baboon_crossing: baboon_cross.c
	$(CC) $(CFLAGS) -o baboon_cross baboon_cross.c