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

/*
============GLOBAL VARIABLES============
*/

#define BUFF_SIZE 256

struct client_info
{
    int sock;
    char id[24];
    char name[24];
    int logged;
};

struct client_info clients[64];
int num_clients = 0;

/*
==============FUNCTIONALITY============
*/

// Check the validity of the message.
// Return 1 if valid, 0 if invalid.
int isValid(char *msg)
{
    // Create a copy of the message to prevent modifying it.
    char msgCopy[BUFF_SIZE];
    strcpy(msgCopy, msg);

    // Count word in message. The number of words must be less than 3.
    int wordCount = 0;
    char *token = strtok(msgCopy, " ");

    // Check a colon at the end of the word.
    if (token[strlen(token) - 1] != ':')
    {
        return 0;
    }

    while (token != NULL)
    {
        wordCount++;
        token = strtok(NULL, " ");
    }

    if (wordCount > 2)
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
void forwardMsg(int sender_num, char *msg)
{
    // Time string.
    char time[24];
    char newMsg[BUFF_SIZE * 2];

    getTimeString(time, sizeof(time));

    // Write formatted message.
    sprintf(newMsg, "%s %s: %s", time, clients[sender_num].id, msg);

    // Send to all other clients.
    for (int i = 0; i < num_clients; i++)
    {
        // Not resend to sender and unlogged-in clients.
        if ((i != sender_num) && (clients[i].logged))
        {
            send(clients[i].sock, newMsg, strlen(newMsg), 0);
        }
    }
}

// Login to chat server.
void login(int sender_num, char *msg)
{
    // Assign id
    char *token = strtok(msg, ": ");
    strncpy(clients[sender_num].id, token, sizeof(clients[sender_num].id));

    // Assign name.
    token = strtok(NULL, ": ");
    strncpy(clients[sender_num].name, token, sizeof(clients[sender_num].name));

    // Change status.
    clients[sender_num].logged = 1;
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

    fd_set fdread;

    // Greeting message.
    char greetMsg[] = "Enter: (id: name)";
    char buf[BUFF_SIZE];

    while (1)
    {
        // Xóa tất cả socket trong tập fdread
        FD_ZERO(&fdread);

        // Thêm socket listener vào tập fdread
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;

        // Thêm các socket client vào tập fdread
        for (int i = 0; i < num_clients; i++)
        {
            FD_SET(clients[i].sock, &fdread);
            if (maxdp < clients[i].sock + 1)
                maxdp = clients[i].sock + 1;
        }

        // Chờ đến khi sự kiện xảy ra
        int ret = select(maxdp, &fdread, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        // Kiểm tra sự kiện có yêu cầu kết nối
        if (FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            printf("New client: %d\n", client);

            // Save the socket.
            clients[num_clients++].sock = client;

            // Send greeting message.
            send(client, greetMsg, strlen(greetMsg), 0);
        }

        // Kiểm tra sự kiện có dữ liệu truyền đến socket client
        for (int i = 0; i < num_clients; i++)
        {
            if (FD_ISSET(clients[i].sock, &fdread))
            {
                ret = recv(clients[i].sock, buf, sizeof(buf), 0);

                if (ret <= 0)
                {
                    // Client đã ngắt kết nối, xóa client ra khỏi mảng
                    close(clients[i].sock);

                    // Change status.
                    clients[i].logged = 0;

                    if (i < num_clients - 1)
                    {
                        clients[i] = clients[num_clients - 1];
                    }
                    num_clients--;

                    continue;
                }

                buf[ret] = 0;

                // Not logged in
                if (clients[i].logged != 1)
                {
                    // Check correct msg
                    if (isValid(buf))
                    {
                        // Login successful
                        login(i, buf);
                    }
                    else
                    {
                        // Wrong pattern - Login failed
                        send(clients[i].sock, greetMsg, sizeof(greetMsg), 0);

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