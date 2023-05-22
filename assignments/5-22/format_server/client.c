#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <poll.h>

#define BUFF_SIZE 256

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("connect() failed");
        return 1;
    }

    /*
    Cau truc pollfd: Tập hợp các mô tả cần đợi sự kiện
        int fd: Mô tả (socket) cần thăm dò
        short int events: Mặt nạ sự kiện cần kiểm tra.
        short int revents: Mặt nạ sự kiện đã xảy ra.
    */
    struct pollfd fds[2];
    char buf[BUFF_SIZE];

    fds[0].fd = STDIN_FILENO; // Describe of input device
    fds[0].events = POLLIN;

    fds[1].fd = client; // Descirbe of client socket
    fds[1].events = POLLIN;

    while (1)
    {
        /*
        Hàm poll: đợi trên 1 tập mô tả
        cho đến khi các thao tác vào ra sẵn sàng.
            struct pollfd *fds: Tập hợp các mô tả cần đợi sự kiện.
            nfds_t nfds: Số lượng các mô tả, ko vượt quá RLIMIT_NOFILE
            int timeout: Thời gian chờ (ms). -1 thì hàm chỉ trả về kết quả
                khi có sự kiện xảy ra.
        */
        int ret = poll(fds, 2, -1);

        // Kiểm tra sự kiện có dữ liệu từ bàn phím
        if (fds[0].revents & POLLIN)
        {
            fgets(buf, sizeof(buf), stdin);
            // if (strncmp(buf, "exit", 4) == 0)
            // {
            //     break;
            // }
            send(client, buf, strlen(buf), 0);
        }

        // Kiểm tra sự kiện có dữ liệu từ socket
        if (fds[1].revents & POLLIN)
        {
            ret = recv(client, buf, sizeof(buf), 0);
            if (ret <= 0)
                break;

            buf[ret] = 0;
            printf("%s\n", buf);
        }
    }

    close(client);
    return 0;
}