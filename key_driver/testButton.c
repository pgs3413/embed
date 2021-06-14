#include "Ex3_API.h"

#define ADD 1 //按下音量加键返回的值是1
#define CUT 2 //按下音量减键返回的值是2，这个需要在驱动程序中定义

#define BUTTON ("/dev/key") //按键目录
#define SEGMENT_DISPLAY ("/dev/FS4412_segdis") //数码管目录


int main(int argc, char* argv) {
	int value = 0; //这个存储当前数码管显示的值

	uint8_t cmd[9]; //控制数码管的参数
	cmd[0] = 9;//cmd[0]设置为9，意思是控制所有的8个数码管

	int fd_segdis = open(SEGMENT_DISPLAY, O_WRONLY); //打开数码管
	if (fd_segdis < 0) {
		printf("open segdis failed\n");
		exit(-1);
	}

	int fd_button = open(BUTTON, O_RDONLY); //打开按键
	if (fd_button < 0) {
		printf("open button failed\n");
		exit(-1);
	}

	uint8_t keyVal[1]; //这个存储从按键读取来的值
	while (1) {
		if (read(fd_button, keyVal, sizeof(int))!= sizeof(int)) { //读值
			printf("read button failed\n");
			exit(-1);
		}

		if (keyVal[0] == ADD) { //ADD就数码管加1
			value = value + 1;
		}
		if (keyVal[0] == CUT) { //CUT就数码管减1
			value = value - 1;
		}

		printf("keyVal is %d\n", keyVal[0]); //屏幕上打印出数码管要显示的值

		int temp = value;
		int i=8;

		//将value中的数字转换成字符存储在cmd中
		char dictionary[10]={'0','1','2','3','4','5','6','7','8','9'};
		for (i = 8; i > 0; i--) {
			cmd[i] = dictionary[temp % 10];
			temp = temp / 10;
		}

		//打印出cmd
		for (i = 1; i < 9; i++) {
			printf("%c", cmd[i]);
		}
		printf("\n");

		//控制数码管显示value的值
		FS4412_segdis_w_num(fd_segdis, cmd);

	}

	//关闭数码管和按键
	close(fd_button);
	close(fd_segdis);

	return 0;
}

