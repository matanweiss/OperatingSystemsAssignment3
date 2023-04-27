CC=gcc
FLAGS=-Wall -g -pthread -o
COMMUNICATIONS=client.c server.c threadFunctions.c

all: stnc

stnc: stnc.c $(COMMUNICATIONS) communications.h 
	$(CC) $(FLAGS) stnc stnc.c $(COMMUNICATIONS)
clean:
	stnc