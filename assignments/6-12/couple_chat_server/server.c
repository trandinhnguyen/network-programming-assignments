#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

int users[2];
int num_users = 0;

// Greeting message.
char greetMsg[] = "Enter ID (e.g. 'id: xyz')";

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

        if (num_users == 2)
        {
            // refuse connection
            close(client);
        }
        else
        {
            printf("New client connected: %d\n", client);
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, client_thread, &client);
            pthread_detach(thread_id);
            send(client, greetMsg, strlen(greetMsg), 0);
        }
    }

    close(listener);
    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[256], id[32], tmp[32];
    int ret;
    users[num_users] = client;
    num_users++;

    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            break;
        }

        buf[ret] = 0;

        printf("Received: %s", buf);

        for (int i = 0; i < 2; i++)
        {
            if (users[i] != client)
            {
                send(users[i], buf, strlen(buf), 0);
            }
        }

        // if (strncmp(buf, "exit", 4) == 0)
        // {
        //     break;
        // }
    }

    // close(users[0]);
    // close(users[1]);
    close(client);
    printf("Client disconnectd\n");
}