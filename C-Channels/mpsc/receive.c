#include "internal.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

t_receiver	*chan_receiver_new(t_channel *channel)
{
	t_receiver	*receiver;

	receiver = ft_calloc(1, sizeof(t_receiver));
	if (!receiver)
		return (NULL);
	pthread_mutex_lock(&channel->mu);
	if (channel->has_receiver == true)
		return (pthread_mutex_unlock(&channel->mu), free(receiver), NULL);
	channel->has_receiver = true;
	pthread_mutex_unlock(&channel->mu);
	receiver->channel_q = channel;
	return (receiver);
}

static t_chan_status	chan_pop(t_channel *chan, void **data)
{
	t_chan_node		*node;
	t_chan_status	status;

	status = CH_CLOSED;
	pthread_mutex_lock(&chan->mu);
	while (chan->head == NULL && chan->closed == false)
		pthread_cond_wait(&chan->not_empty, &chan->mu);
	node = chan->head;
	if (node != NULL)
	{
		chan->head = node->next;
		status = CH_OK;
	}
	if (chan->head == NULL)
		chan->tail = NULL;
	pthread_mutex_unlock(&chan->mu);
	*data = NULL;
	if (node != NULL)
		*data = node->data;
	free(node);
	return (status);
}

t_chan_result	mpsc_recv(t_receiver *receiver)
{
	t_chan_result	result;

	result.data = NULL;
	result.status = chan_pop(receiver->channel_q, &result.data);
	return (result);
}

void	receiver_free(t_receiver *receiver)
{
	chan_destroy(receiver->channel_q);
	free(receiver);
}
