#include "../codexion.h"
#include <stddef.h>
#include <stdlib.h>

void	free_coders(t_coder *coders, unsigned int num)
{
	size_t	i;

	i = 0;
	while (i < num)
	{
		mpsc_sender_drop(coders[i].log_sender);
		i++;
	}
	free(coders);
}

void	free_dongles(t_dongle **dongles, unsigned int num)
{
	size_t	i;

	i = 0;
	while (i < num)
	{
		pthread_cond_destroy(&((*dongles)[i]).state);
		pthread_mutex_destroy(&((*dongles)[i]).mutex);
		i++;
	}
	free(*dongles);
}

void	free_all(t_world_data *world_data)
{
	t_chan_result	rcv;

	free_dongles(&world_data->dongles, world_data->args->number_of_coders);
	free(world_data->coders);
	free(world_data->last_compile_time_arr);
	free(world_data->compilations_done);
	while (1)
	{
		rcv = mpsc_recv(world_data->log_rcv);
		free(rcv.data);
		if (rcv.status == CH_CLOSED)
			break ;
	}
	receiver_free(world_data->log_rcv);
	pthread_mutex_destroy(&world_data->world_mutex);
	free(world_data);
}
