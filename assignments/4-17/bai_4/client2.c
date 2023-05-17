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

// Increase buffer size to reduce packets lost
#define BUFF_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Nhap sai cau lenh!\n");
        return 1;
    }

    // Initialize socket
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Declare address structure
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8888);

    // Read file
    char *filename = argv[1];
    FILE *f = fopen(filename, "rb");

    char buf[BUFF_SIZE];

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
        {
            break;
        }

        sendto(sender, buf, ret, 0, (struct sockaddr *)&addr, sizeof(addr));

        // Decrease send speed to reduce packets lost
        usleep(50000);
    }

    // Close file and socket
    fclose(f);
    close(sender);
}