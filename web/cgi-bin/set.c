#include "main.h"


int main() {


    int fd_pwm = open("/dev/pwm", O_RDWR | O_NONBLOCK);

    int level=0;
	
	scanf("%d",&level);

    ioctl(fd_pwm, SET_CNT,&level);

    close(fd_pwm);
  

}



