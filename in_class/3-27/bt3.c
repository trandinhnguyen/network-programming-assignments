#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char str[] = "227 Entering Passive Mode (213,229,112,130,216,4)";

    int ip[4];
    int port[2];

    char *pc = strchr(str, '(');
    char *p = strtok(pc, "(,)");

    printf("%s\n", p);

    ip[0] = atoi(p);
    for (int i = 1; i <= 3; i++)
    {
        p = strtok(NULL, "(,)");
        ip[i] = atoi(p);
        printf("%s\n", p);
    }

    for (int i = 0; i < 2; i++)
    {
        p = strtok(NULL, "(,)");
        port[i] = atoi(p);
        printf("%s\n", p);
    }

    printf("IP: %u.%u.%u.%u:%u\n", ip[0], ip[1], ip[2], ip[3], port[0] * 256 + port[1]);
}