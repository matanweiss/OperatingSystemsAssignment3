#include "communications.h"

int createServerSocket(int port, int ipType, int isUDP)
{
    struct sockaddr_in serverAddressIPv4;
    struct sockaddr_in6 serverAddressIPv6;
    struct sockaddr_un serverAddressUDS;
    int serverSocket;
    if (isUDP)
        serverSocket = socket(ipType, SOCK_DGRAM, IPPROTO_UDP);
    else
        serverSocket = socket(ipType, SOCK_STREAM, IPPROTO_TCP);

    // If the socket is not established the method sock() return the value -1 (INVALID_SOCKET)
    if (serverSocket == -1)
    {
        perror("socket");
        return -1;
    }

    // Often the activation of the method bind() falls with the message "Address already in use".
    int yes = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        return -1;
    }

    int bindResult = 0;

    if (ipType == AF_INET)
    {
        memset(&serverAddressIPv4, 0, sizeof(serverAddressIPv4));
        serverAddressIPv4.sin_family = ipType;
        serverAddressIPv4.sin_addr.s_addr = INADDR_ANY;
        serverAddressIPv4.sin_port = htons(port);
        bindResult = bind(serverSocket, (struct sockaddr *)&serverAddressIPv4, sizeof(serverAddressIPv4));
    }
    else if (ipType == AF_INET6)
    {
        memset(&serverAddressIPv6, 0, sizeof(serverAddressIPv6));
        serverAddressIPv6.sin6_family = ipType;
        serverAddressIPv6.sin6_port = htons(port);
        bindResult = bind(serverSocket, (struct sockaddr *)&serverAddressIPv6, sizeof(serverAddressIPv6));
    }
    else if (ipType == AF_UNIX)
    {
        memset(&serverAddressUDS, 0, sizeof(serverAddressUDS));
        serverAddressUDS.sun_family = ipType;
        strncpy(serverAddressUDS.sun_path, "uds", sizeof(serverAddressUDS.sun_path) - 1);
        unlink("uds");
        bindResult = bind(serverSocket, (struct sockaddr *)&serverAddressUDS, sizeof(serverAddressUDS));
    }

    // On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
    if (bindResult == -1)
    {
        printf("Bind() failed with error code : %d\n", errno);
        return -1;
    }

    if (!isUDP && listen(serverSocket, 1) == -1)
    {
        printf("listen() failed with error code : %d\n", errno);
        return -1;
    }
    printf("The server is listening...\n\n");
    return serverSocket;
}

int startChatServer(int port)
{
    // create a TCP Connection
    int chatSocket = createServerSocket(port, AF_INET, 0);
    if (chatSocket == -1)
        return -1;

    // infinite loop for the incoming requests
    while (1)
    {
        printf("Waiting for a client to connect...\n\n");
        // accept and incoming connection:
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);

        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);

        int clientSocket = accept(chatSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
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
                int result = got_user_input(clientSocket);
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
                int result = got_client_input(clientSocket);
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

int startInfoServer(int port, int quiet)
{
    // create a TCP Connection
    int infoSocket = createServerSocket(port, AF_INET, 0);
    if (infoSocket == -1)
        return -1;

    // infinite loop for the incoming requests
    while (1)
    {
        printf("Waiting for a client to connect...\n\n");
        // accept and incoming connection:
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);

        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);

        int clientChatSocket = accept(infoSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientChatSocket == -1)
        {
            perror("accept() failed");
            return -1;
        }
        printf("The server is conected\n");

        // getting the type and param comunication
        int ipType, isUDP;
        char typeToPrint[50];
        if (recv(clientChatSocket, &ipType, sizeof(int), 0) < 0)
        {
            perror("recv() failed");
            return -1;
        }
        if (recv(clientChatSocket, &isUDP, sizeof(int), 0) < 0)
        {
            perror("recv() failed");
            return -1;
        }
        if (recv(clientChatSocket, typeToPrint, 50, 0) < 0)
        {
            perror("recv() failed");
            return -1;
        }
        // getting the hash
        unsigned char clientHash[MD5_DIGEST_LENGTH];
        if (recv(clientChatSocket, clientHash, MD5_DIGEST_LENGTH, 0) < 0)
        {
            perror("recv() failed");
            return -1;
        }

        int newPort = 12000;
        if (port == 12000)
            newPort = 13000;
        int dataSocket = createServerSocket(newPort, ipType, isUDP);
        if (dataSocket == -1)
        {
            close(clientChatSocket);
            close(infoSocket);
            return -1;
        }
        int clientDataSocket;
        struct sockaddr_in clientDataAddressIPv4;
        struct sockaddr_in6 clientDataAddressIPv6;
        socklen_t clientDataAddressLenIPv4 = sizeof(clientDataAddressIPv4);
        socklen_t clientDataAddressLenIPv6 = sizeof(clientDataAddressIPv6);

        memset(&clientDataAddressIPv4, 0, sizeof(clientDataAddressIPv4));
        memset(&clientDataAddressIPv6, 0, sizeof(clientDataAddressIPv6));
        clientDataAddressLenIPv4 = sizeof(clientDataAddressIPv4);
        clientDataAddressLenIPv4 = sizeof(clientDataAddressIPv6);
        if (isUDP)
        {
            clientDataSocket = dataSocket;
        }
        else
        {
            if (ipType == AF_INET)
            {
                clientDataSocket = accept(dataSocket, (struct sockaddr *)&clientDataAddressIPv4, &clientDataAddressLenIPv4);
            }
            else if (ipType == AF_INET6)
            {
                clientDataSocket = accept(dataSocket, (struct sockaddr *)&clientDataAddressIPv6, &clientDataAddressLenIPv6);
            }
            if (clientDataSocket == -1)
            {
                perror("accept() failed");
                return -1;
            }
            printf("The server is conected\n");
        }

        int nfds = 2;
        struct pollfd pfds[2];
        // save the stdin file in the poll file descriptor
        pfds[0].fd = clientChatSocket;
        pfds[0].events = POLLIN;
        pfds[1].fd = clientDataSocket;
        pfds[1].events = POLLIN;

        char buffer[BUFFER_SIZE];
        struct timeval start, end;
        FILE *fd = fopen("received.txt", "w");
        int bytesReveived = 0;
        printf("Receiving the: \n");
        while (1)
        {
            printf("Receiving the file: \n");

            poll(pfds, nfds, -1);
            if (pfds[0].revents & POLLIN)
            {
                // int result = got_chat_input(clientChatSocket);
                // if (result == -1)
                // {
                //     printf("got_chat_input() failed\n");
                //     break;
                // }
                // else if (result == 1)
                //     break;
                char chatBuffer[20];
                if (recv(clientChatSocket, chatBuffer, 20, 0) < 0)
                {
                    perror("recv() failed");
                    return -1;
                }
                if (!strcmp(chatBuffer, "start"))
                    gettimeofday(&start, NULL);
                if (!strcmp(chatBuffer, "exit"))
                {
                    gettimeofday(&end, NULL);
                    double timeDelta = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
                    printf("%s, %f\n", typeToPrint, timeDelta);
                    break;
                }
                printf("%s\n", chatBuffer);
            }
            if (pfds[1].revents & POLLIN)
            {
                // int result = got_data_input(clientDataSocket, buffer, &clientDataAddress, &clientDataAddressLen);
                // if (result == -1)
                // {
                //     printf("got_data_input() failed\n");
                //     break;
                // }
                // printf("%s\n", buffer);
                // else if (result == 1)
                //     break;
                int n = 0;
                if (ipType == AF_INET)
                    n = recvfrom(clientDataSocket, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&clientDataAddressIPv4, &clientDataAddressLenIPv4);
                else if (ipType == AF_INET6)
                    n = recvfrom(clientDataSocket, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&clientDataAddressIPv6, &clientDataAddressLenIPv6);
                printf("received %d bytes\n", n);
                fwrite(buffer, n, 1, fd);
                bytesReveived += n;
            }
        }
        close(dataSocket);
        fclose(fd);
    }
    close(infoSocket);
    return 0;
}
