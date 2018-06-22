#include "include/mchatv1.h"
#include "src/mchatv1_structs.h"
#include "src/mchatv1_formatter.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *mesg = "Hello World! How are you?\nThis is a long message to display.";
int main()
{
	mchat_t *mchat = mchatv1_init(NULL);
	mchatv1_set_nickname(mchat, "sean");
	mchatv1_connect(mchat, NULL);
	char buf[1 << 15];
	memset(buf, 0, 1 << 15);
	mchatv1_send_message(mchat, mesg);
	int i = mchatv1_format_mesg(mchat->send_thread, buf);
	printf("Size: %d\n%s", i, buf);
	printf("\n\nActual Size: %d\n", strlen(buf));
	return 0;
}
