
#include "Ex3_API.h"

int main(void)
{
	uint8_t cmd[9];

	int i, cas, tmp, fd_segdis = open(SEGMENT_DISPLAY, O_WRONLY);

	while (1)
	{
		printf("\n\nFS4412_segment_display Control:\n");
		printf("[1]: clear segment displays\n");
		printf("[2]: display 1 digit\n");
		printf("[3]: display 8 numbers\n\n\n\n");
		printf("[q]: Quit\n\n");
		printf("Please select: ");

		cas = 0;
		scanf("%d", &cas);
		printf("\n\n");
		if (cas == 1)
		{
			cmd[0] = 0;
			cmd[1] = 0;
			FS4412_segdis_clr(fd_segdis, cmd);
		}
		else if (cas == 2)
		{
			printf("Please input [position(1~8) digit(0~F)]: ");
			scanf("%d %c", &tmp, &cmd[1]);
	
			cmd[0] = tmp;
			if (cmd[0] > 0 && cmd[0] <= 8 &&
				(cmd[1] >= '0' && cmd[1] <= '9' ||
					cmd[1] >= 'A' && cmd[1] <= 'F' ||
					cmd[1] >= 'a' && cmd[1] <= 'f'))
						FS4412_segdis_w_dig(fd_segdis, cmd);

		}
		else if (cas == 3)
		{
			cmd[0] = 9;
			printf("Please input 8 number(0~F): ");
	
			while (getchar() != '\n');
			for (i = 1; i <= 8; i++)
			{
				scanf("%c", &cmd[i]);
				if (cmd[i] >= '0' && cmd[i] <= '9'
					|| cmd[i] >= 'A' && cmd[i] <= 'F'
					|| cmd[i] >= 'a' && cmd[i] <= 'f')
					continue;
				return 0;
			}
			FS4412_segdis_w_num(fd_segdis, cmd);
		}
		else
			break;
	}
	return 0;
}
