#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

// Server nhan file tu client

#define BUFF_SIZE 4096

int main(int argc, char *argv[])
{
    // Khoi tao socket
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (receiver == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Khai bao cau truc dia chi
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8888);

    // Gan dia chi socket
    if (bind(receiver, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // Khai bao cau truc dia chi sender
    struct sockaddr_in senderAddr;
    int senderAddrLen = sizeof(senderAddr);

    // Receive file name
    char filename[256];
    int name_size, ret;

    recvfrom(receiver, &name_size, sizeof(int), 0, (struct sockaddr *)&senderAddr, &senderAddrLen);
    recvfrom(receiver, filename, name_size, 0, (struct sockaddr *)&senderAddr, &senderAddrLen);
    filename[name_size] = 0;

    // Create file (if not exist) and write file
    char buf[BUFF_SIZE];
    sprintf(buf, "%s_%s", inet_ntoa(senderAddr.sin_addr), filename);
    FILE *f = fopen(buf, "wb");

    // Receive data
    while (1)
    {
        ret = recvfrom(receiver, buf, sizeof(buf), 0, (struct sockaddr *)&senderAddr, &senderAddrLen);
        if (ret <= 0)
        {
            break;
        }

        if (ret < sizeof(buf))
        {
            buf[ret] = 0;
        }

        // Write data into file
        fwrite(buf, 1, ret, f);
    }

    fclose(f);
    close(receiver);
}