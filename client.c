#include "communications.h"

int startClient(char *IP, char *PORT)
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
    int port = atoi(PORT);
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

    pthread_t thread1, thread2;
    int id;
    // Create thread 1 to wait for user input
    id = pthread_create(&thread1, NULL, wait_for_user_input, (void *)&sock);
    if (id != 0)
    {
        printf("Error creating thread 1\n");
        return 1;
    }
    // Create thread 2 to wait for remote data
    id = pthread_create(&thread2, NULL, wait_for_remote_data, (void *)&sock);
    if (id != 0)
    {
        printf("Error creating thread 2\n");
        return 1;
    }
    // Wait for both threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    close(sock);
    return 0;
}