/*
===========CLIENT GUI/NHAN TIN NHAN DEN SERVER===========
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
#define BUFF_SIZE 256

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
    if (argc != 3)
    {
        printf("Nhap sai cau lenh!\n");
        return 1;
    }

    // Khoi tao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Chuyen doi kieu du lieu cua port
    uint16_t port;
    if (!str_to_uint16(argv[2], &port))
    {
        printf("Port khong hop le!\n");
        return 1;
    }

    // Khai bao cau truc sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(port);

    // Ket noi den server
    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("connect() failed");
        return 1;
    }

    // Nhận tin nhắn chào mừng từ server
    char buf[BUFF_SIZE];
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    printf("\nReceived from server: %s\n\n", buf);

    // Client bat dau gui tin nhan
    char msg[BUFF_SIZE];
    int len;

    while (1)
    {
        printf("Nhap tin nhan: ");
        fgets(msg, sizeof(msg), stdin);
        len = sizeof(msg);

        // Nhap 'exit' de dong ket noi
        if (strncmp(msg, "exit", 4) == 0)
        {
            break;
        }

        // Gui tin nhan sang server
        send(client, msg, len, 0);
    }

    // Dong ket noi
    close(client);
}