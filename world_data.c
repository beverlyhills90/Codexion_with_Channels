#include "codexion.h"
#include <stddef.h>
#include <stdlib.h>

static int	arrays_init(t_world_data **world_data, t_arguments *args)
{
	(*world_data)->last_compile_time_arr = ft_calloc(args->number_of_coders,
			sizeof(long long));
	if (!(*world_data)->last_compile_time_arr)
		return (free(*world_data), 1);
	(*world_data)->compilations_done = ft_calloc(args->number_of_coders,
			sizeof(unsigned int));
	if (!(*world_data)->compilations_done)
		return (free((*world_data)->last_compile_time_arr),
			free(*world_data), 1);
	(*world_data)->dongles = dongles_init(args->number_of_coders);
	if (!(*world_data)->dongles)
		return (free((*world_data)->last_compile_time_arr),
			free((*world_data)->compilations_done), free(*world_data), 1);
	return (0);
}

static int	mutexes_fail(t_world_data **world_data, t_arguments *args)
{
	mpsc_sender_drop((*world_data)->log_sender_original);
	receiver_free((*world_data)->log_rcv);
	free((*world_data)->last_compile_time_arr);
	free((*world_data)->compilations_done);
	return (free_dongles(&(*world_data)->dongles, args->number_of_coders),
		free(*world_data), 1);
}

static int	coders_fail(t_world_data **world_data, t_arguments *args)
{
	pthread_mutex_destroy(&(*world_data)->world_mutex);
	mpsc_sender_drop((*world_data)->log_sender_original);
	free((*world_data)->compilations_done);
	free((*world_data)->last_compile_time_arr);
	receiver_free((*world_data)->log_rcv);
	return (free_dongles(&(*world_data)->dongles, args->number_of_coders),
		free(*world_data), 1);
}

int	world_data_init(t_world_data **world_data, t_arguments *args)
{
	*world_data = ft_calloc(1, sizeof(t_world_data));
	if (!*world_data)
		return (1);
	(*world_data)->args = args;
	if (arrays_init(world_data, args) != 0)
		return (1);
	(*world_data)->time_of_start = get_ms();
	if (ch_init(world_data) != 0)
		return (1);
	if (mutexes_init(world_data, args) != 0)
		return (mutexes_fail(world_data, args));
	(*world_data)->coders = coders_init(args, (*world_data)->dongles,
			(*world_data));
	if (!(*world_data)->coders)
		return (coders_fail(world_data, args));
	return ((*world_data)->is_running = RUNNING, 0);
}
