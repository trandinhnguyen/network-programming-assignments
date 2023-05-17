#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

// Client gửi file sang server

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
    if (argc != 4)
    {
        printf("Nhap sai cau lenh!\n");
        return 1;
    }

    // Chuyen doi kieu du lieu cua port
    uint16_t port;
    if (!str_to_uint16(argv[2], &port))
    {
        printf("Port khong hop le!\n");
        return 1;
    }

    // Initialize socket
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Declare address structure
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(port);

    // Read file
    char *filename = argv[3];
    FILE *f = fopen(filename, "rb");

    char buf[2048];

    // Send file name
    int name_size = strlen(filename);
    sendto(sender, &name_size, sizeof(int), 0, (struct sockaddr *)&addr, sizeof(addr));
    strcpy(buf, filename);
    sendto(sender, buf, strlen(filename), 0, (struct sockaddr *)&addr, sizeof(addr));

    // Send file data
    while (!feof(f))
    {
        int ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        sendto(sender, buf, ret, 0, (struct sockaddr *)&addr, sizeof(addr));
    }

    // Close file and socket
    fclose(f);
    close(sender);
}