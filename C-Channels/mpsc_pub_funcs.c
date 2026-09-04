#include "channels.h"
#include "helpers.h"
#include "mpsc/cmpsc.h"
#include <stdlib.h>

t_mpsc	*mpsc_new(void)
{
	t_mpsc		*res;
	t_channel	*chan;
	t_receiver	*receiver;
	t_sender	*sender;

	chan = chan_new();
	if (!chan)
		return (NULL);
	receiver = chan_receiver_new(chan);
	if (!receiver)
		return (chan_destroy(chan), NULL);
	sender = chan_sender_new(chan);
	if (!sender)
		return (free(receiver), chan_destroy(chan), NULL);
	res = ft_calloc(1, sizeof(t_mpsc));
	if (!res)
		return (mpsc_sender_drop(sender), free(receiver),
			chan_destroy(chan), NULL);
	res->receiver = receiver;
	res->sender = sender;
	return (res);
}

void	mpsc_free(t_mpsc *mpsc)
{
	receiver_free(mpsc->receiver);
	free(mpsc);
}
