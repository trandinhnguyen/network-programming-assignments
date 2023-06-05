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

char errorMsg[] = "Wrong command!";
/*
==============FUNCTIONALITY============
*/
void checkFormat(char *format_str, int *format_num)
{
    if (strncmp(format_str, "dd/mm/yyyy", 10) == 0)
    {
        *format_num = 1;
        return;
    }
    if (strncmp(format_str, "dd/mm/yy", 8) == 0)
    {
        *format_num = 2;
        return;
    }
    if (strncmp(format_str, "mm/dd/yyyy", 10) == 0)
    {
        *format_num = 3;
        return;
    }
    if (strncmp(format_str, "mm/dd/yy", 8) == 0)
    {
        *format_num = 4;
        return;
    }
}

int isValid(char *buf, int *format_num)
{
    char tmp[BUFF_SIZE];
    strcpy(tmp, buf);

    char *token = strtok(tmp, " ");
    if (strncmp(token, "GET_TIME", 8) != 0)
    {
        return 0;
    }

    token = strtok(NULL, " ");
    checkFormat(token, format_num);

    if (*format_num == 0)
    {
        return 0;
    }

    token = strtok(NULL, " ");
    if (token != NULL)
    {
        return 0;
    }

    return 1;
}

char getTimeString(char *buf, int *format_num)
{
    time_t t = time(NULL);

    // Lấy thời gian hiện tại theo múi giờ địa phương
    struct tm tm = *localtime(&t);

    memset(buf, 0, BUFF_SIZE);
    switch (*format_num)
    {
    case 1:
        strftime(buf, BUFF_SIZE, "%d/%m/%Y", &tm);
        break;
    case 2:
        strftime(buf, BUFF_SIZE, "%d/%m/%y", &tm);
        break;
    case 3:
        strftime(buf, BUFF_SIZE, "%m/%d/%Y", &tm);
        break;
    case 4:
        strftime(buf, BUFF_SIZE, "%m/%d/%y", &tm);
        break;

    default:
        break;
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

        printf("New client connected: %d\n", client);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);
    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[BUFF_SIZE];
    int format_num = 0;

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            break;
        }

        buf[ret] = 0;
        if (!isValid(buf, &format_num))
        {
            send(client, errorMsg, strlen(errorMsg), 0);
        }
        else
        {
            getTimeString(buf, &format_num);
            send(client, buf, strlen(buf), 0);
        }
    }

    close(client);
}
// GET_TIME dd/mm/yyyy
// GET_TIME dd/mm/yy
// GET_TIME mm/dd/yyyy
// GET_TIME mm/dd/yy