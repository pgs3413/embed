/*
 * pwm_motor.c : test demo driver
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "fs4412_pwm.h"

int main()
{
	int dev_fd;
	int i = 0;

	dev_fd = open("/dev/pwm",O_RDWR | O_NONBLOCK);
	if ( dev_fd == -1 ) {
		perror("open");
		exit(1);
	}

	while(1) {
		printf("rate level: %d\n", i);
		ioctl(dev_fd, SET_CNT, &i);
		getchar();
		if ( i == 2) {
			i = 0;
			continue;
		}
		i++;
	}

	return 0;
}

