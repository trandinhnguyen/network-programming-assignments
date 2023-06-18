/*
===========CLIENT GUI/NHAN DU LIEU DEN SERVER===========
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

// Kich thuoc du lieu
#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
    // Khoi tao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai bao cau truc sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8888);

    // Ket noi den server
    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char msg[BUFF_SIZE];
    int ret;
    FILE *f;

    if ((f = fopen("text.txt", "r")) == NULL)
    {
        printf("Error! Can't open file\n");
        exit(1);
    }

    while (!feof(f))
    {
        ret = fread(msg, 1, sizeof(msg), f);
        if (ret <= 0)
            break;
        send(client, msg, ret, 0);
    }

    // Dong ket noi
    fclose(f);
    close(client);
}