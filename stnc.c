#include <string.h>
#include "communications.h"

int main(int argc, char *argv[])
{
    if (argc < 3 || 4 < argc)
    {
        printf("usage:  ./stnc -c [IP] [PORT]\n\t\tor\n\t./stnc -s [PORT]\n");
        return 1;
    }
    if (!strcmp(argv[1], "-c") && argc == 4)
    {
        startClient(argv[2], argv[3]);
    }
    else if (!strcmp(argv[1], "-s") && argc == 3)
    {
        startServer(argv[2]);
    }
    else
    {
        printf("usage:  ./stnc -c [IP] [PORT]\n\t\tor\n\t./stnc -s [PORT]\n");
        return 1;
    }
    return 0;
}