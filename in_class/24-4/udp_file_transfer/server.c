#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

int main(int argc, char *argv[])
{
    // Tao UDP socket
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Khai bao dia chi ben nhan
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(receiver, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed.\n");
        return 1;
    }

    char buf[2048];

    printf("Waiting for filesize and name...\n");

    // Doc kich thuoc va ten file
    int ret = recvfrom(receiver, buf, sizeof(buf), 0, NULL, NULL);

    long filesize;
    char filename[256] = "new_";

    memcpy(&filesize, buf, sizeof(filesize));
    strcat(filename, buf + sizeof(filesize));
    FILE *file = fopen(filename, "wb");

    printf("Start receiving %s\n", filename);
    long total_count = ceil(filesize / sizeof(buf));
    long count = 0;
    while (1)
    {
        ret = recvfrom(receiver, buf, sizeof(buf), 0, NULL, NULL);
        fwrite(buf, 1, ret, file);
        count++;

        if (ret <= 0)
        {
            printf("No data received\n");
            break;
        }

        if (count == (total_count + 1))
        {
            printf("Reach count limit\n");
            break;
        }
    }
    printf("Finish receiving %s after %ld times\n", filename, count);

    fclose(file);
    close(receiver);
    return 0;
}