#include "cmpsc.h"
#include <stdbool.h>

t_channel	*chan_new(void)
{
	t_channel	*chan;

	chan = ft_calloc(1, sizeof(t_channel));
	if (!chan)
		return (NULL);
	chan->has_receiver = false;
	if (pthread_mutex_init(&chan->mu, NULL) != 0)
		return (free(chan), NULL);
	if (pthread_cond_init(&chan->not_empty, NULL) != 0)
		return (pthread_mutex_destroy(&chan->mu), free(chan), NULL);
	chan->closed = false;
	return (chan);
}

void	chan_close(t_channel *chan)
{
	t_chan_node	*curr;
	t_chan_node	*tmp;

	pthread_mutex_lock(&chan->mu);
	chan->closed = true;
	pthread_cond_broadcast(&chan->not_empty);
	curr = chan->head;
	while (curr != NULL)
	{
		tmp = curr->next;
		free(curr);
		curr = tmp;
	}
	chan->head = NULL;
	chan->tail = NULL;
	pthread_mutex_unlock(&chan->mu);
}

void	chan_destroy(t_channel *chan)
{
	chan_close(chan);
	pthread_mutex_destroy(&chan->mu);
	pthread_cond_destroy(&chan->not_empty);
	free(chan);
}
