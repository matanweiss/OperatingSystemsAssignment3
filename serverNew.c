#include "communications.h"

int createServerSocket(int port, int ipType, int isUDP)
{
    struct sockaddr_in serverAddress;
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

    // the "memset" function copies the character "\0"
    memset(&serverAddress, 0, sizeof(serverAddress));

    // AF_INET is an Address Family that is Internet Protocol IPv4 addresses
    serverAddress.sin_family = AF_INET;
    // any IP at this port (any address to accept incoming messages)
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    // the "htons" convert the port to network endian (big endian)
    serverAddress.sin_port = htons(port);

    // Often the activation of the method bind() falls with the message "Address already in use".
    int yes = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        return -1;
    }

    // Link an address and port with a socket is carried out by the method bind().
    int bindResult = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
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
        struct sockaddr clientDataAddress;
        socklen_t clientDataAddressLen = sizeof(clientDataAddress);

        memset(&clientDataAddress, 0, sizeof(clientDataAddress));
        clientDataAddressLen = sizeof(clientDataAddress);
        if (isUDP)
        {
            clientDataSocket = dataSocket;
        }
        else
        {
            clientDataSocket = accept(dataSocket, (struct sockaddr *)&clientDataAddress, &clientDataAddressLen);
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

        // char message[BUFFER_SIZE];
        char buffer[BUFFER_SIZE];
        struct timeval start, end;

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
                char buffer[20];
                if (recv(clientChatSocket, buffer, 20, 0) < 0)
                {
                    perror("recv() failed");
                    return -1;
                }
                if (!strcmp(buffer, "start"))
                    gettimeofday(&start, NULL);
                if (!strcmp(buffer, "exit"))
                {
                    gettimeofday(&end, NULL);
                    double timeDelta = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
                    printf("%s, %f\n", typeToPrint, timeDelta);
                    break;
                }
                printf("%s\n", buffer);
            }
            if (pfds[1].revents & POLLIN)
            {
                int result = got_data_input(clientDataSocket, buffer, &clientDataAddress, &clientDataAddressLen);
                if (result == -1)
                {
                    printf("got_data_input() failed\n");
                    break;
                }
                printf("%s\n", buffer);
                // else if (result == 1)
                //     break;
            }
        }
    }
    return 0;
}
