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

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Nhap sai cau lenh!\n");
        return 1;
    }

    // Chuẩn bị tin nhắn chào mừng từ file
    char greetingMsg[BUFF_SIZE];
    FILE *f;

    // Mở file
    if ((f = fopen(argv[2], "r")) == NULL)
    {
        printf("Error! Can't open file\n");
        exit(1);
    }

    // Bắt đầu đọc dữ liệu từ file
    fgets(greetingMsg, BUFF_SIZE, f);
    fclose(f);

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
    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("\nClient IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    printf("Send '%s' to client\n", greetingMsg);

    // Gửi tin nhắn chào mừng đến client
    int ret = send(client, greetingMsg, strlen(greetingMsg), 0);
    if (ret != -1)
    {
        printf("%d bytes are sent to client\n\n", ret);
    }

    // Chuẩn bị tin nhắn nhận từ client
    char msg[BUFF_SIZE];

    // Chuẩn bị ghi tin nhắn ra file
    f = fopen(argv[3], "w");
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

        // Ghi ra file
        fprintf(f, "%s", msg);
        printf("Receive from client: %s", msg);
    }

    // Đóng file, đóng kết nối
    fclose(f);
    close(client);
    close(listener);

    return 0;
}