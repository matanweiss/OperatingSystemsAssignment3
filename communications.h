#pragma once

#define _GNU_SOURCE
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
#include <poll.h>
#include <sys/time.h>
#include <time.h>
#include <openssl/md5.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define UDS_PATH "/tmp/uds2"
#define FILE_SIZE 1024 * 1024 * 100
#define SEND_FILE_NAME "message.txt"
#define BUFFER_SIZE 1024
#define MMAP 5
#define PIPE 6

int createServerSocket(int port, int ipType, int isUDP);
int createClientSocketIPv4(char *ip, int port, int ipType, int isUDP, struct sockaddr_in *Address);
int createClientSocketIPv6(char *ip, int port, int ipType, int isUDP, struct sockaddr_in6 *Address);
int createClientPipe(FILE *fd, char *param);
int createServerPipe(FILE *fd, char *filename);
int startChatClient(char *ip, int port);
int startChatServer(int port);

int startInfoClient(char *ip, int port, char *type, char *param);
int startInfoServer(int port, int quiet);

int checkPerformance(char *type, char *param, int *ipType, int *isUDP, char *typeToPrint);

int got_user_input(int);
int got_client_input(int);

// int got_chat_input(int);
int got_data_input(int socket, char buffer[BUFFER_SIZE], struct sockaddr *clientAddress, socklen_t *lenAddress);

unsigned long hash(FILE *str);
