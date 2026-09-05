#include "../codexion.h"

t_coder	*coders_init(t_argumnets *args, t_dongle *dongles,
		t_worldData *world_data)
{
	size_t	i;
	t_coder	*coders;

	i = 0;
	coders = ft_calloc(args->number_of_coders, sizeof(t_coder));
	if (!coders)
		return (NULL);
	while (i < args->number_of_coders)
	{
		coders[i].args = args;
		coders[i].coder_id = i + 1;
		coders[i].left = &dongles[i];
		coders[i].right = &dongles[(i + 1) % args->number_of_coders];
		coders[i].world_data = world_data;
		coders[i].lastComplieTimestomp = 0;
		coders[i].log_sender = mpsc_sender_clone(world_data->log_sender_oiginal);
		if (!coders[i].log_sender)
		    return (NULL); //TODO add cleanup
		i++;
	}
	return (coders);
}

int	coders_create(t_coder *coders, size_t num, t_worldData *world_data)
{
	size_t	i;
	size_t	j;
	int		err;

	i = 0;
	j = 0;
	while (i < num)
	{
		err = pthread_create(&coders[i].thread_id,
				NULL, coders_routine, &coders[i]);
		if (err != 0)
		{
			//safe_world_stop(world_data);
			while (j < i)
			{
				pthread_join(coders[j].thread_id, NULL);
				j++;
			}
			return (1);
		}
		i++;
	}
	return (0);
}