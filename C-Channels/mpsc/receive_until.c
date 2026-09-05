#include "internal.h"
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

static void	take_head(t_channel *chan, t_chan_node **node,
		t_chan_status *status)
{
	*node = chan->head;
	if (*node != NULL)
	{
		chan->head = (*node)->next;
		*status = CH_OK;
	}
	if (chan->head == NULL)
		chan->tail = NULL;
}

static t_chan_status	pop_until(t_channel *chan, void **data,
		struct timespec ts)
{
	t_chan_node		*node;
	t_chan_status	status;
	int				err;

	status = CH_CLOSED;
	err = 1;
	pthread_mutex_lock(&chan->mu);
	while (chan->head == NULL && chan->closed == false && err != ETIMEDOUT)
		err = pthread_cond_timedwait(&chan->not_empty, &chan->mu, &ts);
	take_head(chan, &node, &status);
	if (node == NULL && err == ETIMEDOUT)
		status = CH_TIMEOUT;
	pthread_mutex_unlock(&chan->mu);
	*data = NULL;
	if (node != NULL)
		*data = node->data;
	free(node);
	return (status);
}

t_chan_result	mpsc_recv_until(t_receiver *rcv, long long deadline_ms)
{
	t_chan_result	result;
	struct timespec	ts;

	ts.tv_sec = deadline_ms / 1000;
	ts.tv_nsec = (deadline_ms % 1000) * 1000000;
	result.data = NULL;
	result.status = pop_until(rcv->channel_q, &result.data, ts);
	return (result);
}
