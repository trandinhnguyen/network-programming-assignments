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
#include <ctype.h>

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
char *trimwhitespace(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

void replace_multi_space_with_single_space(char *str)
{
    char *dest = str; /* Destination to copy to */

    /* While we're not at the end of the string, loop... */
    while (*str != '\0')
    {
        /* Loop while the current character is a space, AND the next
         * character is a space
         */
        while (*str == ' ' && *(str + 1) == ' ')
            str++; /* Just skip to next character */

        /* Copy from the "source" string to the "destination" string,
         * while advancing to the next character in both
         */
        *dest++ = *str++;
    }

    /* Make sure the string is properly terminated */
    *dest = '\0';
}

void capitalize(char *str)
{
    // capitalize first character of words
    for (int i = 0; str[i] != '\0'; i++)
    {
        // check first character is lowercase alphabet
        if (i == 0)
        {
            if ((str[i] >= 'a' && str[i] <= 'z'))
                str[i] = str[i] - 32; // subtract 32 to make it capital
            continue;                 // continue to the loop
        }
        if (str[i] == ' ') // check space
        {
            // if space is found, check next character
            ++i;
            // check next character is lowercase alphabet
            if (str[i] >= 'a' && str[i] <= 'z')
            {
                str[i] = str[i] - 32; // subtract 32 to make it capital
                continue;             // continue to the loop
            }
        }
        else
        {
            // all other uppercase characters should be in lowercase
            if (str[i] >= 'A' && str[i] <= 'Z')
                str[i] = str[i] + 32; // subtract 32 to make it small/lowercase
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
                char greetMsg[BUFF_SIZE];
                sprintf(greetMsg, "Hello, there are %d connecting clients!", (nfds - 1));
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
                else
                {
                    buf[ret] = 0;
                    if (strncmp(buf, "exit", 4) == 0)
                    {
                        char byeMsg[] = "Bye!";
                        send(current_client, byeMsg, strlen(byeMsg), 0);
                        close(current_client);

                        if (i < nfds - 1)
                        {
                            fds[i] = fds[nfds - 1];
                        }

                        nfds--;
                        i--;
                        continue;
                    }
                    strcpy(buf, trimwhitespace(buf));
                    replace_multi_space_with_single_space(buf);
                    capitalize(buf);
                    send(current_client, buf, strlen(buf), 0);
                }
            }
        }
    }

    close(listener);
    return 0;
}