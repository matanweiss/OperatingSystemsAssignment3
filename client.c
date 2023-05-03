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