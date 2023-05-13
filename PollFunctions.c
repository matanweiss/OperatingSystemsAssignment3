#include "communications.h"

int got_user_input(int sock)
{
    char buffer[1024];

    scanf("%s", buffer);
    // Sending the file
    int bytesSent;
    bytesSent = send(sock, buffer, strlen(buffer), 0);

    if (bytesSent == -1)
    {
        printf("send() failed with error code : %d\n", errno);
        return -1;
    }
    if (bytesSent == 0)
    {
        printf("peer has closed the TCP connection prior to send().\n");
        return -1;
    }
    // if the function didn't send all the data:
    if (bytesSent < strlen(buffer))
    {
        printf("Only %d bytes sent of %ld bytes \n", bytesSent, strlen(buffer));
        return -1;
    }
    printf("The message sent successfully\n");
    if (!strcmp(buffer, "exit"))
        return 1;
    return 0;
}

int got_client_input(int sock)
{
    char buffer[1024];
    int bytesReceived = recv(sock, buffer, 1024, 0);
    // If the method returns -1 then there is an error.
    if (bytesReceived == -1)
    {
        printf("recv failed with error code : %d", errno);
        return -1;
    }
    // If the function returns 0 it reflects that the connection is closed.
    if (bytesReceived == 0)
    {
        printf("Sender disconnect, exit.\n");
        return -1;
    }
    buffer[bytesReceived] = '\0';
    printf("\nmessage received: %s\n", buffer);
    if (!strcmp(buffer, "exit"))
        return 1;
    return 0;
}

int got_data_input(int socket, char buffer[BUFFER_SIZE], struct sockaddr *clientAddress, socklen_t *lenAddress)
{
    printf("got_data_input function activated\n");
    int n = recvfrom(socket, buffer, BUFFER_SIZE,
                     MSG_WAITALL, clientAddress, lenAddress);
    if (n <= 0)
        return -1;
    buffer[n] = '\0';
    return 0;
}

unsigned long hash(FILE *fd)
{
    unsigned long hash = 5381;
    int c;
    for (size_t i = 0; i < FILE_SIZE - 1; i++)
    {
        c = getc(fd);
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    fseek(fd, 0, SEEK_SET);

    return hash;
}
