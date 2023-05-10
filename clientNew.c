#include "communications.h"

int createClientSocket(char *ip, int port, int ipType, int isUDP)
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
    if (inet_pton(ipType, (const char *)ip, &Address.sin_addr) <= 0)
    {
        printf("%s\n", ip);
        perror("inet_pton() failed");
        return -1;
    }
    // connect to receiver
    if (connect(sock, (struct sockaddr *)&Address, sizeof(Address)) == -1)
    {
        perror("connect() failed");
        close(sock);
        return -1;
    }

    printf("connected to receiver\n");

    return sock;
}

int startChatClient(char *ip, int port)
{
    printf("starting the client\n");
    // creating a new socket
    int clientSocket = createClientSocket(ip, port, AF_INET, 0);
    if (clientSocket == -1)
        return -1;

    int nfds = 2;
    struct pollfd pfds[2];

    // save the stdin file in the poll file descriptor
    pfds[0].fd = STDIN_FILENO;
    pfds[1].fd = clientSocket;
    pfds[0].events = POLLIN;
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
    close(clientSocket);
    return 0;
}

int startInfoClient(char *ip, int port, char *type, char *param)
{
    printf("starting the client\n");
    // creating a new socket
    int clientSocket = createClientSocket(ip, port, AF_INET, 0);
    if (clientSocket == -1)
        return -1;

    // send the type and param to the server, in order to start another connection
    char massege[10];
    size_t i = 0;
    while (type[i] != 0)
    {
        massege[i] = type[i];
        i++;
    }
    size_t j = 0;
    massege[i] = '_';
    i++;
    while (param[j] != 0)
    {
        massege[i] = param[j];
        i++;
        j++;
    }
    massege[i] = 0;
    printf("%s\n", massege);
    send(clientSocket, massege, strlen(massege), 0);

    return 0;
}