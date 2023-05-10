#include "communications.h"

int startClient(char *IP, int port)
{
    printf("starting the client\n");
    // creating a new socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Handle case where couldn't create a socket
    if (sock == -1)
    {
        printf("Could not create socket : %d\n", errno);
        return -1;
    }
    struct sockaddr_in receiverAddress;
    memset(&receiverAddress, 0, sizeof(receiverAddress));
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(port);

    int rval = inet_pton(AF_INET, (const char *)IP, &receiverAddress.sin_addr);
    if (rval <= 0)
    {
        printf("inet_pton() failed\n");
        return -1;
    }

    // Make a connection to the receiver with socket SendingSocket.
    int connectResult = connect(sock, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));
    if (connectResult == -1)
    {
        printf("connect() failed with error code : %d\n", errno);
        // cleanup the socket;
        close(sock);
        return -1;
    }
    printf("connected to the server\n");

    int nfds = 2;
    struct pollfd pfds[2];

    // save the stdin file in the poll file descriptor
    pfds[0].fd = STDIN_FILENO;
    pfds[1].fd = sock;
    pfds[0].events = POLLIN;
    pfds[1].events = POLLIN;

    while (1)
    {
        printf("Enter a message: \n");

        poll(pfds, nfds, -1);
        if (pfds[0].revents & POLLIN)
        {
            int result = got_user_input(&sock);
            if (result == -1)
            {
                printf("got_user_input() failed\n");
                break;
            }
            else if (result == 1)
                break;
        }
        if (pfds[1].revents & POLLIN)
        {
            int result = got_client_input(&sock);
            if (result == -1)
            {
                printf("got_client_input() failed\n");
                break;
            }
            else if (result == 1)
                break;
        }
    }
    close(sock);
    return 0;
}

int startClientPerformance(char *ip, int port, char *type, char *param)
{
    // int sock;
    int isUDP = 0;
    int ipType = 0;
    if (!strcmp(type, "ipv4") && !strcmp(param, "tcp"))
    {
        ipType = AF_INET;
        // sock = socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (!strcmp(type, "ipv4") && !strcmp(param, "udp"))
    {
        // sock = socket(AF_INET, SOCK_DGRAM, 0);
        ipType = AF_INET;
        isUDP = 1;
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "tcp"))
    {
        ipType = AF_INET6;
        // sock = socket(AF_INET6, SOCK_STREAM, 0);
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "udp"))
    {
        ipType = AF_INET6;
        // sock = socket(AF_INET6, SOCK_DGRAM, 0);
        isUDP = 1;
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "dgram"))
    {
        ipType = AF_UNIX;
        // sock = socket(AF_UNIX, SOCK_DGRAM, 0);
        isUDP = 1;
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "stream"))
    {
        ipType = AF_UNIX;
        // sock = socket(AF_UNIX, SOCK_STREAM, 0);
    }
    else if (!strcmp(type, "mmap"))
    {
    }
    else if (!strcmp(type, "pipe"))
    {
    }
    else
    {
        printf("only the following types and params are allowed:\n");
        printf("ipv4 tcp\n");
        printf("ipv4 udp\n");
        printf("ipv6 tcp\n");
        printf("ipv6 udp\n");
        printf("uds dgram\n");
        printf("uds stream\n");
        printf("mmap [filename]\n");
        printf("pipe [filename]\n");
        return -1;
    }

    return sendFile2(port, ip, isUDP, ipType);
}

// int sendFile(int sock, int port, char *ip)
// {
//     struct sockaddr_in Address;
//     memset(&Address, 0, sizeof(Address));

//     Address.sin_family = AF_INET;
//     Address.sin_port = htons(port);
//     int inetResult = inet_pton(AF_INET, (const char *)ip, &Address.sin_addr);
//     if (inetResult <= 0)
//     {
//         perror("inet_pton() failed");
//         return -1;
//     }

//     // connect to receiver
//     int connectResult = connect(sock, (struct sockaddr *)&Address, sizeof(Address));
//     if (connectResult == -1)
//     {
//         perror("connect() failed");
//         close(sock);
//         return -1;
//     }

//     printf("connected to receiver\n");

//     char message[FILE_SIZE];
//     for (size_t i = 0; i < FILE_SIZE; i++)
//     {
//         message[i] = 'H';
//     }
//     message[FILE_SIZE - 1] = '/0';
//     if (send(sock, message, sizeof(char) * FILE_SIZE, 0) == -1)
//     {
//         perror("send() failed");
//         return -1;
//     }
//     printf("message sent\n");
// }

int sendFile2(int port, char *ip, int isUDP, int ipType)
{
    int sock;
    if (isUDP)
        sock = socket(ipType, SOCK_DGRAM, IPPROTO_UDP);
    else
        sock = socket(ipType, SOCK_STREAM, IPPROTO_TCP);

    if (sock == -1)
    {
        perror("socket() failed");
        return -1;
    }
    struct sockaddr_in Address;
    memset(&Address, 0, sizeof(Address));

    Address.sin_family = ipType;
    Address.sin_port = htons(port);
    int inetResult = inet_pton(ipType, (const char *)ip, &Address.sin_addr);
    if (inetResult <= 0)
    {
        printf("%s\n", ip);
        perror("inet_pton() failed");
        return -1;
    }
    // connect to receiver
    int connectResult = connect(sock, (struct sockaddr *)&Address, sizeof(Address));
    if (connectResult == -1)
    {
        perror("connect() failed");
        close(sock);
        return -1;
    }

    printf("connected to receiver\n");

    srand(time(NULL)); // Initialization, should only be called once.
    FILE *fd = fopen("message.txt", "w+");
    char message[100 * BUFFER_SIZE];
    for (size_t i = 0; i < FILE_SIZE - 1; i++)
    {
        char randomChar = (rand() % 126) + 1;
        fprintf(fd, "%c", randomChar);
    }
    fprintf(fd, "%c", 0);
    fseek(fd, 0, SEEK_SET);
    for (size_t i = 0; i < FILE_SIZE; i += (100 * BUFFER_SIZE))
    {
        fread(message, 1, 100 * BUFFER_SIZE, fd);
        // message[BUFFER_SIZE - 1] = 0;
        if (send(sock, message, sizeof(message), 0) == -1)
        {
            perror("send() failed");
            return -1;
        }
        bzero(message, BUFFER_SIZE);
    }

    fclose(fd);
    close(sock);
    printf("message sent\n");
    return 0;
}