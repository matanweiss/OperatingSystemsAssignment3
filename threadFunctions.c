#include "communications.h"

int isSecondThreadCompleted = 0;

void *wait_for_user_input(void *arg)
{
    int sock = *(int *)arg;
    char buffer[1024];
    int fd = fileno(stdin);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    printf("Enter a command: ");
    while (1)
    {
        if (isSecondThreadCompleted)
        {
            return NULL;
        }

        int n = scanf("%s", buffer);
        if (n == EOF)
        {
            continue;
        }
        // Sending the file
        int bytesSent;
        bytesSent = send(sock, buffer, strlen(buffer), 0);

        if (bytesSent == -1)
        {
            printf("send() failed with error code : %d\n", errno);
            return NULL;
        }
        if (bytesSent == 0)
        {
            printf("peer has closed the TCP connection prior to send().\n");
            return NULL;
        }
        // if the function didn't send all the data:
        if (bytesSent < strlen(buffer))
        {
            printf("Only %d bytes sent of %ld bytes \n", bytesSent, strlen(buffer));
            return NULL;
        }
        printf("The message sent successfully\n");
        if (!strcmp(buffer, "exit"))
            return NULL;
        printf("Enter a message: ");
    }
    return NULL;
}
void *wait_for_remote_data(void *arg)
{
    int sock = *(int *)arg;
    char buffer[1024];
    int exit = 0;
    while (!exit)
    {
        int bytesReceived = recv(sock, buffer, 1024, 0);
        // If the method returns -1 then there is an error.
        if (bytesReceived == -1)
        {
            printf("recv failed with error code : %d", errno);
            return NULL;
        }
        // If the function returns 0 it reflects that the connection is closed.
        if (bytesReceived == 0)
        {
            printf("Sender disconnect, exit.\n");
            return NULL;
        }
        buffer[bytesReceived] = '\0';
        printf("message received: %s\n", buffer);
        if (!strcmp(buffer, "exit"))
        {
            send(sock, "exit", sizeof(char) * 5, 0);
            isSecondThreadCompleted = 1;
            exit = 1;
        }
    }
    return NULL;
}