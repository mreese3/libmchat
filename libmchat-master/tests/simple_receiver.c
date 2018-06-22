#define _DEFAULT_SOURCE
#include "mchatv1.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

char *mesg = "Hello World!";
int main()
{
	mchat_t *mchat = mchatv1_init(NULL);
	mchatv1_set_nickname(mchat, "sean", 4);
	mchatv1_connect(mchat, NULL);
	char buf[1 << 15];
	while (1)
	{
		mchat_message_t *message;
		printf("getting message\n");
		int i = mchatv1_recv_message(mchat, &message);
		printf("i = %d\n", i);
		if (i)
		{
			int s = mchatv1_message_get_body_size(message);
			mchatv1_message_get_body(message, buf, 1 << 15);
			buf[s] = '\0';
			printf("%s\n", buf);
		}
		else
			printf("No message\n");
		sleep(1);
	}
	return 0;
}
