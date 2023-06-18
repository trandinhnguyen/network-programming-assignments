/*
=============SERVER GUI/NHAN TIN NHAN TU CLIENT==============
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

// Kich thuoc toi da cua tin nhan
#define BUFF_SIZE 1024

#define OUT 0
#define IN 1

// returns number of words in str
unsigned countWords(char *str)
{
    int state = OUT;
    unsigned wc = 0; // word count

    // Scan all characters one by one
    while (*str)
    {
        // If next character is a separator, set the
        // state as OUT
        if (*str == ' ' || *str == '\n' || *str == '\t')
            state = OUT;

        // If next character is not a word separator and
        // state is OUT, then set the state as IN and
        // increment word count
        else if (state == OUT)
        {
            state = IN;
            ++wc;
        }

        // Move to next character
        ++str;
    }

    return wc;
}

int main(int argc, char *argv[])
{
    // Khởi tạo socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Khai báo cấu trúc địa chỉ sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8888);

    // Gán kết nối đến cấu trúc địa chỉ
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // Chờ kết nối từ client
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    // Khai báo cấu trúc địa chỉ của client
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    // Chấp nhận kết nối từ client
    char *clientIP;
    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    clientIP = inet_ntoa(clientAddr.sin_addr);
    printf("\nClient IP: %s:%d\n\n", clientIP, ntohs(clientAddr.sin_port));

    // Chuẩn bị tin nhắn nhận từ client
    char msg[BUFF_SIZE];

    // Bắt đầu nhận tin nhắn
    while (1)
    {
        // Nhận tin nhắn
        int ret = recv(client, msg, sizeof(msg), 0);
        msg[ret] = 0;
        if (ret <= 0)
        {
            break;
        }

        // Dem so luong o dia
        unsigned int diskCount = (countWords(msg) - 1) / 2;

        char *token = strtok(msg, " ");
        printf("Computer's name: %s\n", token);
        printf("Number of disks: %d\n", diskCount);
        token = strtok(NULL, " ");

        while (token != NULL)
        {
            printf("\t%s - ", token);
            token = strtok(NULL, " ");
            printf("%s\n", token);
            token = strtok(NULL, " ");
        }
    }

    // Đóng file, đóng kết nối
    close(client);
    close(listener);

    return 0;
}