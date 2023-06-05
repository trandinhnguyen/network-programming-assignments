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

// Greeting message.
char greetMsg[] = "Enter: (id: name)";

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

        printf("New client connected: %d\n", client);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);

        // Send greeting message.
        send(client, greetMsg, strlen(greetMsg), 0);
    }

    close(listener);
    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[BUFF_SIZE];
    int logged = 0;
    FILE *f;

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
            if (isExist(buf) == 1)
            {
                logged = 1;
            }
            else
            {
                send(client, greetMsg, strlen(greetMsg), 0);
            }
        }
        else
        {
            char cmd_buf[BUFF_SIZE];
            strncpy(cmd_buf, buf, strlen(buf) - 1);
            strcat(cmd_buf, " > out.txt");
            system(cmd_buf);

            char send_buff[BUFF_SIZE];
            f = fopen("out.txt", "r");
            if (f == NULL)
                printf("Can't open out.txt\n");

            while (!feof(f))
            {
                ret = fread(send_buff, 1, sizeof(send_buff), f);
                send(client, send_buff, ret, 0);
            }

            fclose(f);
        }
    }

    close(client);
}