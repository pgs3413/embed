#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include<asm-generic/ioctl.h>

int main(){
    int fd=open("/dev/exp",O_RDWR);
    ioctl(fd,1);
    while(1){

    }
}