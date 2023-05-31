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

    // Greeting message.
    char greetMsg[] = "Enter: (user pass)";

    FILE *f;
    f = fopen("out.txt", "r");
    if (f == NULL)
        printf("Can't open out.txt\n");

    while (1)
    {
        printf("Waiting for new client...\n");
        int client = accept(listener, NULL, NULL);

        if (fork() == 0)
        {
            // Tien trinh con
            close(listener);

            // Xu ly ket noi tu client
            int loged = 0;
            char buf[BUFF_SIZE];
            
            send(client, greetMsg, strlen(greetMsg), 0);

            while (1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;
                buf[ret] = 0;

                if (loged == FALSE)
                {
                    if (isExist(buf) == FALSE)
                    {
                        send(client, greetMsg, strlen(greetMsg), 0);
                    }
                    else
                    {
                        loged = TRUE;
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
                    while (!feof(f))
                    {
                        ret = fread(send_buff, 1, sizeof(send_buff), f);
                        send(client, send_buff, ret, 0);
                    }

                    fclose(f);
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