#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <poll.h>
#include <stdlib.h>

#define BUFF_SIZE 256

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Wrong command");
        return 1;
    }
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in sender_addr;
    sender_addr.sin_family = AF_INET;
    sender_addr.sin_addr.s_addr = inet_addr(argv[1]);
    sender_addr.sin_port = htons(atoi(argv[2]));

    // int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = inet_addr(argv[1]);
    receiver_addr.sin_port = htons(atoi(argv[3]));

    if (bind(sender, (struct sockaddr *)&sender_addr, sizeof(sender_addr)))
    {
        perror("bind() failed.\n");
        return 1;
    }

    struct pollfd fds[2];
    char buf[BUFF_SIZE];

    fds[0].fd = STDIN_FILENO; // Describe of input device
    fds[0].events = POLLIN;

    fds[1].fd = sender; // Descirbe of client socket
    fds[1].events = POLLIN;

    while (1)
    {
        int ret = poll(fds, 2, -1);

        // Kiểm tra sự kiện có dữ liệu từ bàn phím
        if (fds[0].revents & POLLIN)
        {
            fgets(buf, sizeof(buf), stdin);
            if (strncmp(buf, "exit", 4) == 0)
            {
                break;
            }
            sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));
        }

        // Kiểm tra sự kiện có dữ liệu từ socket
        if (fds[1].revents & POLLIN)
        {
            ret = recvfrom(sender, buf, sizeof(buf), 0, NULL, NULL);
            if (ret <= 0)
                break;

            buf[ret] = 0;
            printf("%s\n", buf);
        }
    }

    close(sender);
    // close(receiver);
    return 0;
}