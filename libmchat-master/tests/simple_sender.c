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
	while (1)
	{
		mchatv1_send_message(mchat, mesg);
		sleep(2);
	}
	return 0;
}
