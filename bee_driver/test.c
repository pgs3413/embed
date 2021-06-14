#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "fs4412_bee.h"

int main(int argc, char **argv)
{
    int fd;

    fd = open("/dev/bee", O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    printf("enter any key\n");
    getchar();
    ioctl(fd, BEE_ON);
    getchar();
    ioctl(fd, BEE_OFF);

    return 0;
}
