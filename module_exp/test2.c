#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include<asm-generic/ioctl.h>

#define cmd _IOW('L',0,int)

int main(){
    int fd=open("/dev/exp",O_RDWR);
    if(fd==0) printf("wrong");
    ioctl(fd,cmd);
    while(1){

    }
}