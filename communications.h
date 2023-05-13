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
#include <time.h>
#include <openssl/md5.h>

#define FILE_SIZE 1024 * 1024 * 100
#define BUFFER_SIZE 1024

int createServerSocket(int port, int ipType, int isUDP);
int createClientSocket(char *ip, int port, int ipType, int isUDP,struct sockaddr_in * Address);


int startChatClient(char *ip, int port);
int startChatServer(int port);

int startInfoClient(char *ip, int port, char *type, char *param);
int startInfoServer(int port, int quiet);

int checkPerformance(char *type, char *param, int *ipType, int *isUDP);
int hash_file(FILE *fd, unsigned char *hash);

int got_user_input(int);
int got_client_input(int);

// int got_chat_input(int);
int got_data_input(int socket, char buffer [BUFFER_SIZE], struct sockaddr * clientAddress, socklen_t * lenAddress);

int startServer2(int, int);
int startClientPerformance(char *, int, char *, char *);
int startServerPerformance(int, char *, char *, int);
// int receiveFile(int sock, int isUDP, int port, int quiet, char *typeToPrint);
int receiveFile2(int isUDP, int ipType, int port, int quiet, char *typeToPrint);
// int sendFile(int sock, int port, char *ip);
int sendFile2(int port, char *ip, int isUDP, int ipType);
