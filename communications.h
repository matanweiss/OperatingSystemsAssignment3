#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/time.h>

#define FILE_SIZE 1024 * 1024 * 100
#define BUFFER_SIZE 1024

int startClient(char *, char *);
int startServer(char *);
int startClientPerformance(char *, int, char *, char *);
int startServerPerformance(int, char *, char *, int);
int got_user_input(int *);
int got_client_input(int *);
int receiveFile(int sock, int isUDP, int port, int quiet, char *typeToPrint);
