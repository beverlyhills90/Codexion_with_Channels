#ifndef CMPSC_H
# define CMPSC_H

# include "../helpers.h"
# include <pthread.h>
# include <stdlib.h>

typedef struct s_sender		t_sender;
typedef struct s_receiver	t_receiver;

typedef struct s_chan_node
{
	void				*data;
	struct s_chan_node	*next;
}	t_chan_node;

typedef enum e_chan_status
{
	CH_OK,
	CH_ERR,
	CH_CLOSED
}	t_chan_status;

typedef struct s_chan_result
{
	t_chan_status	status;
	void			*data;
}	t_chan_result;

typedef struct s_channel
{
	pthread_mutex_t	mu;
	pthread_cond_t	not_empty;
	size_t			senders;
	t_chan_node		*head;
	t_chan_node		*tail;
	bool			has_receiver;
	bool			closed;
}	t_channel;

t_sender		*chan_sender_new(t_channel *channel);
t_receiver		*chan_receiver_new(t_channel *channel);
t_sender		*mpsc_sender_clone(t_sender *origin);
void			mpsc_send(t_sender *sender, void *data);
t_chan_result	mpsc_recv(t_receiver *receiver);
void			mpsc_sender_drop(t_sender *sender);

t_channel		*chan_new(void);
void			chan_close(t_channel *chan);
void			chan_destroy(t_channel *chan);
void			receiver_free(t_receiver *receiver);

#endif
