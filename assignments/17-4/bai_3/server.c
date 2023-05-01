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

// Chuyen tu string sang uint16
int str_to_uint16(char *str, uint16_t *port)
{
    char *end;
    errno = 0;

    // Chuyen tu string sang long
    long val = strtol(str, &end, 10);

    if (errno || end == str || *end != '\0' || val < 0 || val >= 0x10000)
    {
        return 0;
    }

    // Chuyen long sang uint16
    *port = (uint16_t)val;
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Nhap sai cau lenh!\n");
        return 1;
    }

    // Chuyen doi kieu du lieu cua port
    uint16_t port;
    if (!str_to_uint16(argv[1], &port))
    {
        printf("Port khong hop le!\n");
        return 1;
    }

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
    addr.sin_port = htons(port);

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
    char buf[2048];
    sprintf(buf, "new_%s", filename);
    FILE *f = fopen(buf, "wb");

    // Receive data
    while (1)
    {
        ret = recvfrom(receiver, buf, sizeof(buf), 0, (struct sockaddr *)&senderAddr, &senderAddrLen);
        if (ret <= 0)
        {
            break;
        }

        // Write data into file
        fwrite(buf, 1, ret, f);
    }

    fclose(f);
    close(receiver);
}