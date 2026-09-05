#include "C-Channels/channels.h"
#include "C-Channels/helpers.h"
#include "codexion.h"
#include <stddef.h>
#include <stdlib.h>

int	mutexes_init(t_worldData **worldData, t_argumnets *args)
{
	if (pthread_mutex_init(&(*worldData)->world_mutex, NULL) != 0)
	{
	    free((*worldData)->lastComplieTimeArr);
		return (free_dongles((*worldData)->dongles, args->number_of_coders),
				free(*worldData), 1);
	}
	if (pthread_mutex_init(&(*worldData)->output_mutex, NULL) != 0)
	{
        free((*worldData)->lastComplieTimeArr);
		pthread_mutex_destroy(&(*worldData)->world_mutex);
		return (free_dongles((*worldData)->dongles, args->number_of_coders),
			free(*worldData), 1);
	}
	return (0);
}

int ch_init(t_worldData **worldData)
{
    t_mpsc *MPSC;

    MPSC = mpsc_new();
    if (!MPSC)
        return (1); //ADD CLEAUP
    (*worldData)->log_rcv = MPSC->receiver;
    (*worldData)->log_sender_oiginal = MPSC->sender;
    return (0);
}

//1 - err 0 - ok
int	world_data_init(t_worldData **worldData, t_argumnets *args)
{
	*worldData = ft_calloc(1, sizeof(t_worldData));
	if (!*worldData)
		return (1);
	(*worldData)->lastComplieTimeArr = ft_calloc(args->number_of_coders, sizeof(long long));
	if (!(*worldData)->lastComplieTimeArr)
	    return (free(*worldData), 1);
	(*worldData)->compilationsDone = ft_calloc(args->number_of_coders, sizeof(unsigned int));
	if (!(*worldData)->lastComplieTimeArr)
	    return (free((*worldData)->lastComplieTimeArr),free(*worldData), 1);
	(*worldData)->dongles = dongles_init(args->number_of_coders);
	if (!(*worldData)->dongles)
		return (free((*worldData)->lastComplieTimeArr), free(*worldData), 1);
	(*worldData)->timeOfStart = get_ms();
	if (mutexes_init(worldData, args) != 0)
		return (1);
	(*worldData)->args = args;
	if (ch_init(worldData) != 0)
	    return (1); //TODO add clean up
	(*worldData)->coders = coders_init(args, (*worldData)->dongles,
			(*worldData));
	if (!(*worldData)->coders)
	{
		pthread_mutex_destroy(&(*worldData)->world_mutex);
		pthread_mutex_destroy(&(*worldData)->output_mutex);
		return (free_dongles((*worldData)->dongles, args->number_of_coders),
			free(*worldData), 1);
	}
	return ((*worldData)->is_running = RUNNING, 0);
}