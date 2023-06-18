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
#define TRUE 1
#define FALSE 0

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

    int num_threads = 8;
    pthread_t thread_ids[num_threads];

    for (int i = 0; i < num_threads; i++)
    {
        int ret = pthread_create(&thread_ids[i], NULL, client_thread, &listener);
        if (ret != 0)
        {
            printf("Could not create new thread.\n");
            sched_yield();
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(thread_ids[i], NULL);
    }

    close(listener);
    return 0;
}

void *client_thread(void *param)
{
    int listener = *(int *)param;
    char buf[BUFF_SIZE];

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        printf("New client %d accepted in thread %ld with pid %d\n", client,
               pthread_self(), getpid());

        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            close(client);
            continue;
        }

        buf[ret] = 0;
        printf("Received: %s\n", buf);

        char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</ h1></ body></ html> ";
        send(client, msg, strlen(msg), 0);
        close(client);
    }
}
