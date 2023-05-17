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

int users[64];      // Logged-in client sockets list
char *user_ids[64]; // Logged-in client ids list
int num_users = 1;

struct pollfd fds[64];
int nfds = 1;

/*
==============FUNCTIONALITY============
*/

// Check the validity of the message.
// Return 1 if valid, 0 if invalid.
int isValid(char *msg, char *id, char *name, char *tmp)
{
    int ret = sscanf(msg, "%s%s%s", id, name, tmp);
    if (ret == 2)
    {
        if (id[strlen(id) - 1] != ':')
            return 0;
    }
    else
    {
        return 0;
    }
    return 1;
}
// Write formatted current time to str.
char getTimeString(char *str, int bufferSize)
{
    // Lấy thời gian hiện tại
    time_t t = time(NULL);

    // Lấy thời gian hiện tại theo múi giờ địa phương
    struct tm tm = *localtime(&t);

    // Write formatted string to str
    snprintf(str, bufferSize, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

// Forward message to other clients.
void forwardMsg(int sender_idx, char *msg)
{
    // Time string.
    char time[24];
    char newMsg[BUFF_SIZE * 2];

    getTimeString(time, sizeof(time));

    // Write formatted message.
    sprintf(newMsg, "%s %s %s", time, user_ids[sender_idx], msg);

    // Send to all other clients.
    for (int i = 1; i < num_users; i++)
    {
        if (i != sender_idx)
        {
            send(users[i], newMsg, strlen(newMsg), 0);
        }
    }
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
    char greetMsg[] = "Enter: (id: name)";
    char buf[BUFF_SIZE];

    fds[0].fd = listener;
    fds[0].events = POLLIN;

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
                        // users[i] = users[nfds - 1];
                    }

                    nfds--;
                    i--;

                    continue;
                }

                buf[ret] = 0;

                int j = 1;
                for (; j < num_users; j++)
                    if (users[j] == current_client)
                    {
                        break;
                    }

                // Not logged in
                if (j == num_users)
                {
                    // Process login command
                    char id[32], name[32], tmp[32];

                    // Message correct
                    if (isValid(buf, id, name, tmp))
                    {
                        // Login successful
                        users[i] = current_client;
                        user_ids[i] = malloc(strlen(id) + 1);
                        strcpy(user_ids[i], id);
                        num_users++;
                    }
                    else
                    {
                        // Wrong pattern - Login failed
                        send(current_client, greetMsg, sizeof(greetMsg), 0);

                        // Move to the next client.
                        continue;
                    }
                }
                else
                {
                    // Logged in
                    // Send message to all other clients.
                    forwardMsg(i, buf);
                }
            }
        }
    }

    close(listener);
    return 0;
}