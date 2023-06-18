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
#define BUFF_SIZE 256

// Chuyển từ string sang uint16
int str_to_uint16(char *str, uint16_t *port)
{
    char *end;
    errno = 0;

    // Chuyển từ string sang long
    long val = strtol(str, &end, 10);

    if (errno || end == str || *end != '\0' || val < 0 || val >= 0x10000)
    {
        return 0;
    }

    // Chuyển long sang uint16
    *port = (uint16_t)val;
    return 1;
}

// Lấy thời gian hiện tại
char *getTimeString()
{
    // Lấy thời gian hiện tại
    time_t t = time(NULL);

    // Lấy thời gian hiện tại theo múi giờ địa phương
    struct tm tm = *localtime(&t);

    // Format chuỗi thời gian
    static char str[24];
    sprintf(str, "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    return str;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Nhap sai cau lenh!\n");
        return 1;
    }

    // Khởi tạo socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Chuyen doi kieu du lieu cua port
    uint16_t port;
    if (!str_to_uint16(argv[1], &port))
    {
        printf("Port khong hop le!\n");
        return 1;
    }

    // Khai báo cấu trúc địa chỉ sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

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
    char time[24];
    char result[2 * BUFF_SIZE];

    // Chuẩn bị ghi tin nhắn ra file
    FILE *f;
    f = fopen(argv[2], "w");
    if (f == NULL)
    {
        printf("Error! Can't open file to write\n");
        exit(1);
    }

    // Bắt đầu nhận tin nhắn và ghi ra file
    while (1)
    {
        // Nhận tin nhắn
        int ret = recv(client, msg, sizeof(msg), 0);
        msg[ret] = 0;
        if (ret <= 0)
        {
            break;
        }

        // Lấy thời gian hiện tại
        strcpy(time, getTimeString());

        // Nối chuỗi
        snprintf(result, sizeof(result), "%s %s%s", clientIP, time, msg);

        // Ghi ra file và in ra màn hình
        fprintf(f, "%s", result);
        printf("%s", result);
    }

    // Đóng file, đóng kết nối
    fclose(f);
    close(client);
    close(listener);

    return 0;
}