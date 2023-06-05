#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define BUFF_SIZE 256

int clients[64];
int num_clients = 1;

// Greeting message.
char greetMsg[] = "Enter: (id: name)";

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
void forwardMsg(int client, char *id, char *msg)
{
    // Time string.
    char time[24];
    char newMsg[BUFF_SIZE * 2];

    getTimeString(time, sizeof(time));

    // Write formatted message.
    sprintf(newMsg, "%s %s %s", time, id, msg);

    // Send to all other clients.
    for (int i = 1; i < num_clients; i++)
    {
        if (client != clients[i])
        {
            send(clients[i], newMsg, strlen(newMsg), 0);
        }
    }
}

void *client_thread(void *);

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

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }

        if (num_clients == 64)
        {
            // Refuse connection
            close(client);
        }
        else
        {
            printf("New client connected: %d\n", client);
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, client_thread, &client);
            pthread_detach(thread_id);
            // Send greeting message.
            send(client, greetMsg, strlen(greetMsg), 0);
        }
    }

    close(listener);
    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[256], id[32], name[32], tmp[32];
    int logged = 0;

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            break;
        }

        buf[ret] = 0;
        if (!logged)
        {
            if (isValid(buf, id, name, tmp))
            {
                clients[num_clients] = client;
                logged = 1;
                num_clients++;
            }
            else
            {
                send(client, greetMsg, sizeof(greetMsg), 0);
            }
        }
        else
        {
            forwardMsg(client, id, buf);
        }
    }

    close(client);
}