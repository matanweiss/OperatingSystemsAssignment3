CC=gcc
FLAGS=-Wall -g -pthread -o
COMMUNICATIONS=clientNew.c serverNew.c PollFunctions.c

all: stnc

stnc: stnc.c $(COMMUNICATIONS) communications.h 
	$(CC) $(FLAGS) stnc stnc.c $(COMMUNICATIONS) -lssl -lcrypto
clean:
	stnc