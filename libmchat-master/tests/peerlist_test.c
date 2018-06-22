#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <glib.h>
#include <string.h>
#include <mchatv1.h>
#include <mchatv1_structs.h>
#include <mchatv1_utils.h>

const char *name = "sean\0";
const char *chan = "#mchat\0";

int main(int argc, char *argv[])
{
	mchat_t *mchat = mchatv1_init(NULL);
	mchat_peer p;
	memset(&p, 0, sizeof(mchat_peer));
	memcpy(p.nickname, name, strlen(name));
	p.nickname_len = strlen(name);
	memcpy(p.channel, chan, strlen(chan));
	p.channel_len = strlen(chan);
	p.last_seen = g_get_real_time();

	g_array_append_val(mchat->peerlist, p);

	mchat_peerlist_t *pl;
	mchatv1_get_peerlist(mchat, &pl);

	int s = mchatv1_peerlist_get_size(pl);
	mchat_peer q = g_array_index(mchat->peerlist, mchat_peer, 0);
	g_print("Peerlist is %d long\n", s);
	g_print("Nickname in mchat: %s (%d)\n", q.nickname, q.nickname_len);
	g_print("Nickname: %s (%d)\n", pl->list[0].nickname, pl->list[0].nickname_len);
	g_print("Channel: %s (%d)\n", pl->list[0].channel, pl->list[0].channel_len);
	g_print("Channel Length here: %d\n", strlen(chan));
	g_print("Last seen %ld\n", pl->list[0].last_seen);
	return 0;
}
