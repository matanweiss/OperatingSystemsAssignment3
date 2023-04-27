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

        pthread_t thread1, thread2;
        int id;
        // Create thread 1 to wait for user input
        id = pthread_create(&thread1, NULL, wait_for_user_input, (void *)&clientSocket);
        if (id != 0)
        {
            printf("Error creating thread 1\n");
            return 1;
        }
        // Create thread 2 to wait for remote data
        id = pthread_create(&thread2, NULL, wait_for_remote_data, (void *)&clientSocket);
        if (id != 0)
        {
            printf("Error creating thread 2\n");
            return 1;
        }
        // Wait for both threads to finish
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);

        // close the connection:
        close(clientSocket);
    }
    return 0;
}
