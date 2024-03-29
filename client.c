#include "communications.h"

int createClientMmap(char *param)
{
    int fd = open(SEND_FILE_NAME, O_RDONLY);
    if (fd < 0)
    {
        perror("File open error");
        return -1;
    }

    struct stat statMmap;
    if (fstat(fd, &statMmap) < 0)
    {
        perror("fstat");
        close(fd);
        return -1;
    }

    size_t size = statMmap.st_size;

    int fdShared = shm_open(param, O_CREAT | O_RDWR, 0666);
    if (fdShared < 0)
    {
        perror("Sshm_open");
        close(fd);
        return -1;
    }

    ftruncate(fdShared, size); // resize the shared mamory to the file size.

    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdShared, 0);
    if (addr == MAP_FAILED)
    {
        perror("mmap: ");
        close(fd);
        shm_unlink(param);
        return -1;
    }
    if (read(fd, addr, size) < 0)
    {

        perror("read");
        close(fd);
    }

    close(fd);
    return 0;
}

int createClientPipe(FILE *fd, char *param)
{
    int fdFIFO;
    unlink(param);

    if (mkfifo(param, 0666) == -1)
    {
        fclose(fd);
        return -1;
    }

    if ((fdFIFO = open(param, O_WRONLY)) == -1)
    {
        fclose(fd);
        unlink(param);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesSent;

    while ((bytesSent = fread(buffer, 1, BUFFER_SIZE, fd)) > 0)
    {
        write(fdFIFO, buffer, bytesSent);
    }

    close(fdFIFO);
    fclose(fd);
    unlink(param);
    return 0;
}

int createClientSocketUDS(char *ip, int port, int ipType, int isUDP, struct sockaddr_un *Address)
{
    int sock;
    if (isUDP)
        sock = socket(ipType, SOCK_DGRAM, 0);
    else
        sock = socket(ipType, SOCK_STREAM, 0);

    if (sock == -1)
    {
        perror("socket() failed");
        return -1;
    }
    memset(Address, 0, sizeof(struct sockaddr_un));
    Address->sun_family = ipType;
    strncpy(Address->sun_path, UDS_PATH, sizeof(Address->sun_path) - 1);
    // unlink(UDS_PATH);
    if (!isUDP && connect(sock, (struct sockaddr *)Address, sizeof(struct sockaddr_un)) == -1)
    {
        perror("connect() failed");
        close(sock);
        return -1;
    }
    return sock;
}

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
        perror("inet_pton() failed");
        return -1;
    }
    if (!isUDP && connect(sock, (struct sockaddr *)Address, sizeof(struct sockaddr_in6)) == -1)
    {
        perror("connect() failed");
        close(sock);
        return -1;
    }
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
    send(clientSocket, typeToPrint, strlen(typeToPrint) + 1, 0);

    // generate 100MB file
    srand(time(NULL)); // Initialization, should only be called once.
    FILE *fd = fopen(SEND_FILE_NAME, "w+");
    for (size_t i = 0; i < FILE_SIZE - 1; i++)
    {
        char randomChar = (rand() % 255) + 1;
        fprintf(fd, "%c", randomChar);
    }
    fprintf(fd, "%c", 0);
    fseek(fd, 0, SEEK_SET);

    unsigned long checksum = hash(fd);
    send(clientSocket, &checksum, sizeof(checksum), 0);

    char buffer[BUFFER_SIZE];
    sleep(1);

    // creating the data socket
    int newPort = 12000;
    if (port == 12000)
        newPort = 13000;
    int senderSocket = 0;
    struct sockaddr_in6 dataAddressIPv6;
    struct sockaddr_in dataAddressIPv4;
    struct sockaddr_un dataAddressUDS;
    if (ipType == AF_INET)
        senderSocket = createClientSocketIPv4(ip, newPort, ipType, isUDP, &dataAddressIPv4);
    else if (ipType == AF_INET6)
        senderSocket = createClientSocketIPv6(ip, newPort, ipType, isUDP, &dataAddressIPv6);
    else if (ipType == AF_UNIX)
        senderSocket = createClientSocketUDS(ip, newPort, ipType, isUDP, &dataAddressUDS);
    // else if (ipType == MMAP)
    //     senderSocket = createClientMMAP(param, fd);
    else if (ipType == PIPE)
    {
        send(clientSocket, param, strlen(param) + 1, 0);

        if (createClientPipe(fd, param) == -1)
        {
            close(clientSocket);
            return -1;
        }
        close(clientSocket);
        return 0;
    }
    else if (ipType == MMAP)
    {
        char paramWithBackslash[strlen(param) + 2];
        paramWithBackslash[0] = '/';
        strcpy(paramWithBackslash + 1, param);
        paramWithBackslash[strlen(param) + 1] = '\0';
        send(clientSocket, paramWithBackslash, strlen(paramWithBackslash) + 1, 0);
        fclose(fd);
        if (createClientMmap(paramWithBackslash) == -1)
        {
            close(clientSocket);
            return -1;
        }
        close(clientSocket);
        return 0;
    }

    if (senderSocket == -1)
    {
        close(clientSocket);
        return -1;
    }

    send(clientSocket, "start", 6, 0);
    for (size_t i = 0; i < FILE_SIZE; i += (BUFFER_SIZE))
    {
        fread(buffer, 1, BUFFER_SIZE, fd);
        int bytesSent = 0;
        if (ipType == AF_INET)
            bytesSent = sendto(senderSocket, buffer, BUFFER_SIZE, MSG_CONFIRM, (struct sockaddr *)&dataAddressIPv4, sizeof(dataAddressIPv4));
        else if (ipType == AF_INET6)
            bytesSent = sendto(senderSocket, buffer, BUFFER_SIZE, MSG_CONFIRM, (struct sockaddr *)&dataAddressIPv6, sizeof(dataAddressIPv6));
        else if (ipType == AF_UNIX)
        {
            if (isUDP)
                bytesSent = sendto(senderSocket, buffer, BUFFER_SIZE, MSG_CONFIRM, (struct sockaddr *)&dataAddressUDS, sizeof(dataAddressUDS));
            else
                bytesSent = send(senderSocket, buffer, BUFFER_SIZE, 0);
        }
        if (0 >= bytesSent)
        {
            perror("sendto() failed");
            return -1;
        }
        bzero(buffer, BUFFER_SIZE);
    }
    fclose(fd);

    printf("the file has been sent\n");

    sleep(1);
    if (isUDP)
        sleep(3);
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
    memset(typeToPrint, 0, 50);
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
        *(ipType) = MMAP;
        strcpy(typeToPrint, "mmap");
    }
    else if (!strcmp(type, "pipe"))
    {
        *(ipType) = PIPE;
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
