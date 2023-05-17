#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <poll.h>

/*
============GLOBAL VARIABLES============
*/

#define BUFF_SIZE 256
#define TRUE 1
#define FALSE 0

struct pollfd fds[64];
int nfds = 1;

int users[64];
int num_users = 1;

/*
==============FUNCTIONALITY============
*/
int isExist(char *buf)
{
    FILE *f;
    f = fopen("account.txt", "r");
    if (f == NULL)
    {
        printf("Can't open account.txt\n");
        return 0;
    }

    char line[32];
    while (!feof(f))
    {
        fgets(line, sizeof(line), f);
        if (strncmp(buf, line, strlen(line)) == 0)
        {
            return 1;
        }
    }
    fclose(f);
    return 0;
}
/*
=========================MAIN============================
*/
int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    // Greeting message.
    char greetMsg[] = "Enter: (user pass)";
    char buf[BUFF_SIZE];

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    FILE *f;
    f = fopen("out.txt", "r");
    if (f == NULL)
        printf("Can't open out.txt\n");

    while (1)
    {
        // Chờ đến khi sự kiện xảy ra
        int ret = poll(fds, nfds, -1);

        if (ret < 0)
        {
            perror("poll() failed");
            return 1;
        }

        // Kiểm tra sự kiện có yêu cầu kết nối
        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);

            if (nfds == 64)
            {
                // Refuse connection
                close(client);
            }
            else
            {
                printf("New client: %d\n", client);

                // Save the socket.
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;

                // Send greeting message.
                send(client, greetMsg, strlen(greetMsg), 0);
            }
        }

        // Kiểm tra sự kiện có dữ liệu truyền đến socket client
        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                int current_client = fds[i].fd;
                ret = recv(current_client, buf, sizeof(buf), 0);

                if (ret <= 0)
                {
                    // Client đã ngắt kết nối, xóa client ra khỏi mảng
                    close(current_client);

                    if (i < nfds - 1)
                    {
                        fds[i] = fds[nfds - 1];
                    }

                    nfds--;
                    i--;
                    continue;
                }

                int j = 1;
                for (; i < num_users; j++)
                    if (current_client == users[j])
                        break;

                if (j == num_users)
                {
                    if (isExist(buf) == FALSE)
                    {
                        // Request client re-entry
                        send(current_client, greetMsg, strlen(greetMsg), 0);
                    }
                    else
                    {
                        users[i] = current_client;
                        num_users++;
                    }
                }
                else
                {
                    char cmd_buf[BUFF_SIZE];
                    buf[ret] = 0;
                    strncpy(cmd_buf, buf, strlen(buf) - 1);
                    strcat(cmd_buf, " > out.txt");
                    system(cmd_buf);

                    char send_buff[BUFF_SIZE];
                    f = fopen("out.txt", "r");
                    while (!feof(f))
                    {
                        ret = fread(send_buff, 1, sizeof(send_buff), f);
                        send(current_client, send_buff, ret, 0);
                    }

                    fclose(f);
                }
            }
        }
    }

    close(listener);
    return 0;
}