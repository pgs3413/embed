#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include<asm-generic/ioctl.h>

#define LENGTH  100
#define MEM_SET_0 0x1
#define LED_MAGIC_1 'L'
#define MEM_SET_1  _IOW(LED_MAGIC_1, 0, int)
#define LED_MAGIC 'R'
#define MEM_CLEAR  _IOW(LED_MAGIC, 0, int)

int main(int argc, char *argv[])
{
	char str[1024];
	int fd,len;
	int error_no;

    fd = open("/dev/globalmem", O_RDWR);

    if (fd) {
		printf("\n\n");
		printf("write hello world to globalmem.\n\n");
		write(fd,"hello world",strlen("hello world"));
		
		lseek(fd,0,SEEK_SET);
		len=read(fd,str,1024);
		str[len]='\0';
		printf("read string from globalmem:  %s\n\n",str);
		
		ioctl(fd,MEM_SET_0);

		lseek(fd,0,SEEK_SET);
		len=read(fd,str,1024);
		str[len]='\0';
		printf("MEM_SET_0:  %s\n\n",str);
		
		ioctl(fd,MEM_SET_1);

		lseek(fd,0,SEEK_SET);
		len=read(fd,str,1024);
		str[len]='\0';
		printf("MEM_SET_1:  %s\n\n",str);

	    lseek(fd,2,SEEK_SET);
		len=read(fd,str,1024);
		str[len]='\0';
		printf("LLSEEK TO 2:  %s\n\n",str);
	
		ioctl(fd,MEM_CLEAR);
		lseek(fd,0,SEEK_SET);
		len=read(fd,str,1024);
		str[len]='\0';
		printf("MEM_CLEAR:  %s\n\n",str);

		printf("end\n\n");

    }else printf("open error");

	close(fd);

    return 0;
}
