#include "communications.h"

int startServer(char *PORT)
{
    // create a TCP Connection
    struct sockaddr_in serverAddress;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // If the socket is not established the method sock() return the value -1 (INVALID_SOCKET)
    if (sockfd == -1)
    {
        perror("socket");
        return -1;
    }

    // the "memset" function copies the character "\0"
    memset(&serverAddress, 0, sizeof(serverAddress));

    // AF_INET is an Address Family that is Internet Protocol IPv4 addresses
    serverAddress.sin_family = AF_INET;
    // any IP at this port (any address to accept incoming messages)
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    // the "htons" convert the port to network endian (big endian)
    int port = atoi(PORT);
    serverAddress.sin_port = htons(port);

    // Often the activation of the method bind() falls with the message "Address already in use".
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        return -1;
    }

    // Link an address and port with a socket is carried out by the method bind().
    int bindResult = bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    // On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
    if (bindResult == -1)
    {
        printf("Bind() failed with error code : %d\n", errno);
        return -1;
    }

    int listenResult = listen(sockfd, 1);
    if (listenResult == -1)
    {
        printf("listen() failed with error code : %d\n", errno);
        return -1;
    }
    printf("The server is listening...\n\n");

    // infinite loop for the incoming requests
    while (1)
    {
        printf("Waiting for a client to connect...\n\n");
        // accept and incoming connection:
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);

        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);

        int clientSocket = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1)
        {
            perror("accept() failed");
            return -1;
        }
        printf("The server is conected\n");

        int nfds = 2;
        struct pollfd pfds[2];
        // save the stdin file in the poll file descriptor
        pfds[0].fd = STDIN_FILENO;
        pfds[0].events = POLLIN;
        pfds[1].fd = clientSocket;
        pfds[1].events = POLLIN;

        while (1)
        {
            printf("Enter a message: \n");

            poll(pfds, nfds, -1);
            if (pfds[0].revents & POLLIN)
            {
                int result = got_user_input(&clientSocket);
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
                int result = got_client_input(&clientSocket);
                if (result == -1)
                {
                    printf("got_client_input() failed\n");
                    break;
                }
                else if (result == 1)
                    break;
            }
        }

        // close the connection:
        close(clientSocket);
    }
    return 0;
}

int startServerPerformance(int port, char *type, char *param, int quiet)
{
    int sock;
    int isUDP = 0;
    char *typeToPrint;
    if (!strcmp(type, "ipv4") && !strcmp(param, "tcp"))
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        typeToPrint = "ipv4_tcp";
    }
    else if (!strcmp(type, "ipv4") && !strcmp(param, "udp"))
    {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        isUDP = 1;
        typeToPrint = "ipv4_udp";
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "tcp"))
    {
        sock = socket(AF_INET6, SOCK_STREAM, 0);
        typeToPrint = "ipv6_tcp";
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "udp"))
    {
        sock = socket(AF_INET6, SOCK_DGRAM, 0);
        isUDP = 1;
        typeToPrint = "ipv6_udp";
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "dgram"))
    {
        sock = socket(AF_UNIX, SOCK_DGRAM, 0);
        isUDP = 1;
        typeToPrint = "uds_dgram";
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "stream"))
    {
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        typeToPrint = "uds_stream";
    }
    else if (!strcmp(type, "mmap"))
    {
        typeToPrint = "mmap";
    }
    else if (!strcmp(type, "pipe"))
    {
        typeToPrint = "pipe";
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
    if (sock == -1)
    {
        perror("socket() failed");
        return 1;
    }
    receiveFile(sock, isUDP, port, quiet, typeToPrint);
    return 0;
}

int receiveFile(int sock, int isUDP, int port, int quiet, char *typeToPrint)
{
    struct sockaddr_in Address;
    memset(&Address, 0, sizeof(Address));

    Address.sin_family = AF_INET;
    Address.sin_addr.s_addr = INADDR_ANY;
    Address.sin_port = htons(port);

    int bindResult = bind(sock, (struct sockaddr *)&Address, sizeof(Address));
    if (bindResult == -1)
    {
        perror("bind() failed");
        close(sock);
        return -1;
    }
    if (!isUDP && listen(sock, 1) == -1)
    {
        perror("listen() failed");
        close(sock);
        return -1;
    }
    while (1)
    {
        // accept incoming connection
        printf("Waiting for connections...\n");
        struct sockaddr_in senderAddress; //
        socklen_t senderAddressLen = sizeof(senderAddress);
        memset(&senderAddress, 0, sizeof(senderAddress));
        senderAddressLen = sizeof(senderAddress);
        int senderSocket = accept(sock, (struct sockaddr *)&senderAddress, &senderAddressLen);
        if (senderSocket == -1)
        {
            perror("accept() failed");
            close(sock);
            return -1;
        }
        printf("client connection accepted\n");

        struct timeval start, end;
        gettimeofday(&start, NULL);
        FILE *fd = fopen('received.txt', 'a');
        char buffer[BUFFER_SIZE] = {0};
        size_t n = 0;
        while (n < FILE_SIZE)
        {
            if ((n += recv(senderSocket, buffer, BUFFER_SIZE, 0)) < 0)
            {
                perror("recv() failed");
                return -1;
            }
            // if (!strcmp(buffer, "exit"))
            // {
            //     printTimes(times);
            //     return 0;
            // }
            fprintf(fd, buffer);
        }
        gettimeofday(&end, NULL);
        if (quiet)
        {
            printf("%s\n", typeToPrint);
        }
    }
    close(sock);
    return 0;
}