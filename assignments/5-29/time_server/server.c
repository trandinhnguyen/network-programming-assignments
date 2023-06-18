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
#include <signal.h>
#include <sys/wait.h>

/*
============GLOBAL VARIABLES============
*/

#define BUFF_SIZE 256
#define TRUE 1
#define FALSE 0

int typeFormat = 0;
/*
==============FUNCTIONALITY============
*/

void checkFormat(char *format)
{
    printf("Checking format\n");
    if (strncmp(format, "dd/mm/yyyy", 10) == 0)
    {
        typeFormat = 1;
        return;
    }
    if (strncmp(format, "dd/mm/yy", 8) == 0)
    {
        typeFormat = 2;
        return;
    }
    if (strncmp(format, "mm/dd/yyyy", 10) == 0)
    {
        typeFormat = 3;
        return;
    }
    if (strncmp(format, "mm/dd/yy", 8) == 0)
    {
        typeFormat = 4;
        return;
    }
}
int isValid(char *buf)
{
    char tmp[BUFF_SIZE];
    strcpy(tmp, buf);

    char *token = strtok(tmp, " ");
    if (strncmp(token, "GET_TIME", 8) != 0)
    {
        return 0;
    }

    token = strtok(NULL, " ");
    checkFormat(token);
    if (typeFormat == 0)
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

char getTimeString(char *buf)
{
    time_t t = time(NULL);

    // Lấy thời gian hiện tại theo múi giờ địa phương
    struct tm tm = *localtime(&t);

    memset(buf, 0, BUFF_SIZE);
    switch (typeFormat)
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
void signalHandler(int signo)
{
    int pid;
    if ((pid = wait(NULL)) != -1)
        printf("Child %d terminated.\n", pid);
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

    signal(SIGCHLD, signalHandler);

    char errorMsg[] = "Wrong command!";

    while (1)
    {
        printf("Waiting for new client...\n");
        int client = accept(listener, NULL, NULL);

        if (fork() == 0)
        {
            // Tien trinh con
            close(listener);

            // Xu ly ket noi tu client
            typeFormat = 0;
            char buf[BUFF_SIZE];

            while (1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;

                buf[ret] = 0;

                if (!isValid(buf))
                {
                    send(client, errorMsg, strlen(errorMsg), 0);
                }
                else
                {
                    getTimeString(buf);
                    send(client, buf, strlen(buf), 0);
                }
            }

            close(client);
            exit(0);
        }

        // Tien trinh cha
        close(client);
    }

    close(listener);
    return 0;
}
// GET_TIME dd/mm/yyyy
// GET_TIME dd/mm/yy
// GET_TIME mm/dd/yyyy
// GET_TIME mm/dd/yy