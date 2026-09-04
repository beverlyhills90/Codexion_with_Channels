#include "internal.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

t_sender	*chan_sender_new(t_channel *channel)
{
	t_sender	*sender;

	sender = ft_calloc(1, sizeof(t_sender));
	if (!sender)
		return (NULL);
	pthread_mutex_lock(&channel->mu);
	channel->senders++;
	pthread_mutex_unlock(&channel->mu);
	sender->channel_q = channel;
	return (sender);
}

t_sender	*mpsc_sender_clone(t_sender *origin)
{
	t_sender	*sender;

	sender = ft_calloc(1, sizeof(t_sender));
	if (!sender)
		return (NULL);
	sender->channel_q = origin->channel_q;
	pthread_mutex_lock(&origin->channel_q->mu);
	origin->channel_q->senders++;
	pthread_mutex_unlock(&origin->channel_q->mu);
	return (sender);
}

static void	chan_push(t_channel *chan, void *data)
{
	t_chan_node	*node;

	node = ft_calloc(1, sizeof(t_chan_node));
	if (!node)
		return ;
	node->data = data;
	pthread_mutex_lock(&chan->mu);
	if (chan->tail == NULL)
	{
		chan->head = node;
		chan->tail = node;
	}
	else
	{
		chan->tail->next = node;
		chan->tail = node;
	}
	pthread_cond_signal(&chan->not_empty);
	pthread_mutex_unlock(&chan->mu);
}

void	mpsc_send(t_sender *sender, void *data)
{
	chan_push(sender->channel_q, data);
}

void	mpsc_sender_drop(t_sender *sender)
{
	pthread_mutex_lock(&sender->channel_q->mu);
	sender->channel_q->senders--;
	if (sender->channel_q->senders == 0)
	{
		sender->channel_q->closed = true;
		pthread_cond_broadcast(&sender->channel_q->not_empty);
	}
	pthread_mutex_unlock(&sender->channel_q->mu);
	free(sender);
}
