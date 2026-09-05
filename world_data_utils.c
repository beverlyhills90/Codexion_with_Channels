#include "codexion.h"
#include <stddef.h>
#include <stdlib.h>

int	mutexes_init(t_world_data **world_data, t_arguments *args)
{
	if (pthread_mutex_init(&(*world_data)->world_mutex, NULL) != 0)
	{
		free((*world_data)->last_compile_time_arr);
		return (free_dongles(&(*world_data)->dongles,
				args->number_of_coders), free(*world_data), 1);
	}
	return (0);
}

int	ch_init(t_world_data **world_data)
{
	t_mpsc	*mpsc;

	mpsc = mpsc_new();
	if (!mpsc)
	{
		free((*world_data)->last_compile_time_arr);
		free((*world_data)->compilations_done);
		free_dongles(&(*world_data)->dongles,
			(*world_data)->args->number_of_coders);
		return (free(*world_data), 1);
	}
	(*world_data)->log_rcv = mpsc->receiver;
	(*world_data)->log_sender_original = mpsc->sender;
	free(mpsc);
	return (0);
}
