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

int startClient(char *, char *);
int startServer(char *);
void *wait_for_user_input(void *);
void *wait_for_remote_data(void *);
int got_user_input(int *);
int got_client_input(int *);
