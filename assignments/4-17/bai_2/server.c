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

// Kich thuoc toi da cua tin nhan
#define BUFF_SIZE 1024

// Counts occurrences of a word in a given string
int countOccurrence(char *str, char *word)
{
    int i, j, count = 0;

    for (i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == word[0])
        {
            for (j = 1; word[j] != '\0'; j++)
            {
                if (str[i + j] != word[j])
                    break;
            }
            if (word[j] == '\0')
                count++;
        }
    }
    return count;
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
    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("\nClient IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // Chuẩn bị tin nhắn nhận từ client
    char msg[BUFF_SIZE];
    char finalStr[] = "";

    // The word that need to be count
    char word[11] = "0123456789";

    // Store the size of result
    int size = 0;

    // Chuẩn bị ghi tin nhắn ra file
    FILE *f;
    f = fopen("log.txt", "w");
    if (f == NULL)
    {
        printf("Error! Can't open file to write\n");
        exit(1);
    }

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

        size += sizeof(msg);

        // Ghi ra file
        fprintf(f, "%s", msg);
    }

    // Dong file
    fclose(f);

    // Reopen the file to read
    f = fopen("log.txt", "r");
    if (f == NULL)
    {
        printf("Error! Can't open file\n");
        exit(1);
    }

    // Count the occurrences of a word
    fgets(finalStr, size, f);
    int count = countOccurrence(finalStr, "0123456789");
    printf("Number of words: %d\n", count);

    // Đóng file, đóng kết nối
    close(client);
    close(listener);

    return 0;
}