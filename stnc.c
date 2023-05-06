#include <string.h>
#include "communications.h"

int main(int argc, char *argv[])
{
    int client = 0;
    int server = 0;
    int performance = 0;
    int quiet = 0;
    int port = 0;
    char *ip;
    char *type;
    char *param;
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-c"))
        {
            if (argc <= i + 2 || server)
            {
                printf("usage: ./stnc -c [IP] [PORT] -p [type] [param]\n");
                return -1;
            }
            client = 1;
            port = atoi(argv[++i]);
            ip = argv[++i];
            continue;
        }
        if (!strcmp(argv[i], "-s"))
        {
            if (argc <= i + 1 || client)
            {
                printf("usage: ./stnc -s [PORT] -p [type] [param] -q\n");
                return -1;
            }
            server = 1;
            port = atoi(argv[++i]);
            continue;
        }
        if (!strcmp(argv[i], "-p"))
        {
            if (argc <= i + 2)
            {
                printf("usage:  ./stnc -c [IP] [PORT] -p [type] [param] -q\n\t\tor\n\t./stnc -s [PORT] -p [type] [param] -q\n");
                return -1;
            }
            performance = 1;
            type = argv[++i];
            param = argv[++i];
            continue;
        }
        if (!strcmp(argv[i], "-q"))
        {

            if (!performance)
            {
                printf("usage:  ./stnc -c [IP] [PORT] -p [type] [param] -q\n\t\tor\n\t./stnc -s [PORT] -p [type] [param] -q\n");
                return -1;
            }
            quiet = 1;
            continue;
        }
    }
    if (client)
    {
        if (performance)
            startClientPerformance(ip, port, type, param);
        else
            startClient(ip, port);
    }
    else if (server)
    {
        if (performance)
            startServerPerformance(port, type, param, quiet);
        else
            startServer(port);
    }
    else
    {
        printf("usage:  ./stnc -c [IP] [PORT] -p [type] [param] -q\n\t\tor\n\t./stnc -s [PORT] -p [type] [param] -q\n");
        return -1;
    }

    // if (argc < 3 || 4 < argc)
    // {
    //     printf("usage:  ./stnc -c [IP] [PORT]\n\t\tor\n\t./stnc -s [PORT]\n");
    //     return 1;
    // }
    // if (!strcmp(argv[1], "-c") && argc == 4)
    // {
    //     startClient(argv[2], argv[3]);
    // }
    // else if (!strcmp(argv[1], "-s") && argc == 3)
    // {
    //     startServer(argv[2]);
    // }
    // else
    // {
    //     printf("usage:  ./stnc -c [IP] [PORT]\n\t\tor\n\t./stnc -s [PORT]\n");
    //     return 1;
    // }
    return 0;
}