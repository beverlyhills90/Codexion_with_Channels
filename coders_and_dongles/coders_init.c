#include "../codexion.h"

t_coder	*coders_init(t_arguments *args, t_dongle *dongles,
		t_world_data *world_data)
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
		coders[i].last_compile_timestamp = 0;
		coders[i].log_sender
			= mpsc_sender_clone(world_data->log_sender_original);
		if (!coders[i].log_sender)
			return (free_coders(coders, i), NULL);
		i++;
	}
	return (coders);
}

int	coders_create(t_coder *coders, size_t num, t_world_data *world_data)
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
			world_stop(world_data);
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
