#include <stdio.h>
#include <string.h>

int main()
{
    char str[256];
    char cmd[16], tmp[16];
    float x, y;

    printf("Nhap lenh: ");
    fgets(str, sizeof(str), stdin);

    int ret = sscanf(str, "%s%f%f%s", cmd, &x, &y, tmp);
    if (ret < 3)
    {
        printf("ERROR nhap thieu tham so\n");
        return 1;
    }

    if (ret > 3)
    {
        printf("ERROR nhap thua tham so\n");
        return 1;
    }

    if (strcmp(cmd, "ADD") != 0 && strcmp(cmd, "SUB") != 0 && strcmp(cmd, "MUL") != 0 && strcmp(cmd, "DIV") != 0)
    {
        printf("ERROR sai cau lenh\n");
        return 1;
    }

    printf("Cau lenh da nhap: %s %.1f %.1f\n", cmd, x, y);
}