#include "communications.h"

int createClientSocketIPv6(char *ip, int port, int ipType, int isUDP, struct sockaddr_in6 *Address)
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
    memset(Address, 0, sizeof(struct sockaddr_in6));
    Address->sin6_family = ipType;
    Address->sin6_port = htons(port);
    if (inet_pton(ipType, (const char *)ip, &Address->sin6_addr) <= 0)
    {
        printf("%s\n", ip);
        perror("inet_pton() failed");
        return -1;
    }
    if (!isUDP && connect(sock, (struct sockaddr *)Address, sizeof(struct sockaddr_in6)) == -1)
    {
        perror("connect() failed");
        close(sock);
        return -1;
    }
    printf("connected to receiver\n");
    return sock;
}

int createClientSocketIPv4(char *ip, int port, int ipType, int isUDP, struct sockaddr_in *Address)
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
    memset(Address, 0, sizeof(struct sockaddr_in));
    Address->sin_family = ipType;
    Address->sin_port = htons(port);
    if (inet_pton(ipType, (const char *)ip, &Address->sin_addr) <= 0)
    {
        printf("%s\n", ip);
        perror("inet_pton() failed");
        return -1;
    }

    // connect to receiver
    if (!isUDP && connect(sock, (struct sockaddr *)Address, sizeof(struct sockaddr_in)) == -1)
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
    struct sockaddr_in Address;
    int clientSocket = createClientSocketIPv4(ip, port, AF_INET, 0, &Address);
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
    close(clientSocket);
    return 0;
}

int startInfoClient(char *ip, int port, char *type, char *param)
{
    printf("starting the client\n");
    // creating the info socket
    struct sockaddr_in chatAddress;
    int clientSocket = createClientSocketIPv4("127.0.0.1", port, AF_INET, 0, &chatAddress);
    if (clientSocket == -1)
        return -1;

    int ipType, isUDP;
    char typeToPrint[50];
    if (checkPerformance(type, param, &ipType, &isUDP, typeToPrint) == -1)
        return -1;
    // send the info to the server before we send the 100MB data
    send(clientSocket, &ipType, sizeof(int), 0);
    send(clientSocket, &isUDP, sizeof(int), 0);
    send(clientSocket, typeToPrint, strlen(typeToPrint), 0);

    // generate 100MB file
    srand(time(NULL)); // Initialization, should only be called once.
    FILE *fd = fopen("message.txt", "w+");
    // char message[100 * BUFFER_SIZE];
    for (size_t i = 0; i < FILE_SIZE - 1; i++)
    {
        char randomChar = (rand() % 255) + 1;
        fprintf(fd, "%c", randomChar);
    }
    fprintf(fd, "%c", 0);

    // hashing the file according to md5 algorithm
    unsigned char hash[MD5_DIGEST_LENGTH];
    // hash_file(fd, hash);
    send(clientSocket, hash, sizeof(MD5_DIGEST_LENGTH), 0);

    // sleep(0.5);

    // creating the data socket
    int newPort = 12000;
    if (port == 12000)
        newPort = 13000;
    int senderSocket = 0;
    struct sockaddr_in6 dataAddressIPv6;
    struct sockaddr_in dataAddressIPv4;
    if (ipType == AF_INET)
    {
        senderSocket = createClientSocketIPv4(ip, newPort, ipType, isUDP, &dataAddressIPv4);
        if (senderSocket == -1)
        {
            close(clientSocket);
            return -1;
        }
    }
    else if (ipType == AF_INET6)
    {
        senderSocket = createClientSocketIPv6(ip, newPort, ipType, isUDP, &dataAddressIPv6);
        if (senderSocket == -1)
        {
            close(clientSocket);
            return -1;
        }
    }

    send(clientSocket, "start", 6, 0);
    char buffer[BUFFER_SIZE];
    fseek(fd, 0, SEEK_SET);
    for (size_t i = 0; i < FILE_SIZE; i += (BUFFER_SIZE))
    {
        int res = fread(buffer, 1, BUFFER_SIZE, fd);
        // message[BUFFER_SIZE - 1] = 0;
        if (ipType == AF_INET)
        {
            if (0 >= sendto(senderSocket, buffer, BUFFER_SIZE, MSG_CONFIRM, (struct sockaddr *)&dataAddressIPv4, sizeof(dataAddressIPv4)))
            {
                perror("send() failed");
                return -1;
            }
        }
        else if (ipType == AF_INET6)
        {
            if (0 >= sendto(senderSocket, buffer, BUFFER_SIZE, MSG_CONFIRM, (struct sockaddr *)&dataAddressIPv6, sizeof(dataAddressIPv6)))
            {
                perror("send() failed");
                return -1;
            }
        }
        bzero(buffer, BUFFER_SIZE);
        printf("res: %d, i: %ld\n", res, i);
    }
    fclose(fd);

    printf("the file has been sent\n");
    // char *message = "hi, helpp\n";
    // if (0 >= sendto(senderSocket, message, strlen(message), MSG_CONFIRM, &dataAddress, sizeof(dataAddress)))
    // {
    //     printf("didnt send the message\n");
    //     return -1;
    // }
    // send the file
    // if (send_file(fd) == -1)
    // {

    // }
    sleep(1);
    send(clientSocket, "exit", 5, 0);
    close(clientSocket);
    close(senderSocket);
    return 0;
}

int checkPerformance(char *type, char *param, int *ipType, int *isUDP, char *typeToPrint)
{
    // int sock;
    *(isUDP) = 0;
    *(ipType) = 0;
    if (!strcmp(type, "ipv4") && !strcmp(param, "tcp"))
    {
        *(ipType) = AF_INET;
        strcpy(typeToPrint, "ipv4_tcp");
    }
    else if (!strcmp(type, "ipv4") && !strcmp(param, "udp"))
    {
        *(ipType) = AF_INET;
        *(isUDP) = 1;
        strcpy(typeToPrint, "ipv4_udp");
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "tcp"))
    {
        *(ipType) = AF_INET6;
        strcpy(typeToPrint, "ipv6_tcp");
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "udp"))
    {
        *(ipType) = AF_INET6;
        *(isUDP) = 1;
        strcpy(typeToPrint, "ipv6_udp");
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "dgram"))
    {
        *(ipType) = AF_UNIX;
        *(isUDP) = 1;
        strcpy(typeToPrint, "uds_dgram");
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "stream"))
    {
        *(ipType) = AF_UNIX;
        strcpy(typeToPrint, "uds_stream");
    }
    else if (!strcmp(type, "mmap"))
    {
        strcpy(typeToPrint, "mmap");
    }
    else if (!strcmp(type, "pipe"))
    {
        strcpy(typeToPrint, "pipe");
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
    return 0;
}
