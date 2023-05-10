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

        int clientSocket = accept(infoSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1)
        {
            perror("accept() failed");
            return -1;
        }
        printf("The server is conected\n");

        // getting the type and param comunication
        char buffer[10] = {0};
        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0)
        {
            perror("recv() failed");
            return -1;
        }
        char type[10];
        size_t i = 0;
        while (buffer[i] != '_')
        {
            type[i] = buffer[i];
            i++;
        }
        type[i] = 0;
        char param[10];
        i++;
        size_t j = 0;
        while (buffer[i] != 0)
        {
            param[j] = buffer[i];
            i++;
            j++;
        }
        param[j] = 0;
    }
    return 0;
}
