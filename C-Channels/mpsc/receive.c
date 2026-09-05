#include "cmpsc.h"
#include "internal.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
# include <errno.h>

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

t_chan_result	mpsc_recv(t_receiver *rcv)
{
	t_chan_result	result;

	result.data = NULL;
	result.status = chan_pop(rcv->channel_q, &result.data);
	return (result);
}

t_chan_status pop_until(t_channel *chan, void **data,struct timespec ts)
{
    t_chan_node		*node;
	t_chan_status	status;
	int err;

	status = CH_CLOSED;
	err = 1;
	pthread_mutex_lock(&chan->mu);
	while (chan->head == NULL && chan->closed == false && err != ETIMEDOUT)
		err = pthread_cond_timedwait(&chan->not_empty, &chan->mu,&ts);
	node = chan->head;
	if (node != NULL)
	{
		chan->head = node->next;
		status = CH_OK;
	}
	else if (err == ETIMEDOUT)
	    status = CH_TIMEOUT;
	if (chan->head == NULL) {
	    chan->tail = NULL;
	}
	
	pthread_mutex_unlock(&chan->mu);
	*data = NULL;
	if (node != NULL)
		*data = node->data;
	free(node);
	return (status);
}

t_chan_result mpsc_recv_until(t_receiver *rcv, long long deadline_ms)
{
    t_chan_result	result;
   	struct timespec	ts;
   
	ts.tv_sec = deadline_ms / 1000;
	ts.tv_nsec = (deadline_ms % 1000) * 1000000;
   
	result.data = NULL;
	result.status = pop_until(rcv->channel_q, &result.data,ts);
	return (result);
}

void	receiver_free(t_receiver *receiver)
{
	chan_destroy(receiver->channel_q);
	free(receiver);
}
