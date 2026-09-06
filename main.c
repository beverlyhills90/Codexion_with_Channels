#include "codexion.h"
#include <stdlib.h>

int	join_coders(t_world_data *world_data)
{
	unsigned long	i;
	int				err;
	int				has_error;

	i = 0;
	has_error = 0;
	while (i < world_data->args->number_of_coders)
	{
		err = pthread_join(world_data->coders[i].thread_id, NULL);
		if (err != 0)
			has_error = 1;
		i++;
	}
	release_senders(world_data);
	if (pthread_join(world_data->monitor_thread_id, NULL) != 0)
		has_error = 1;
	return (has_error);
}

int	run_simulation(t_world_data *world_data)
{
	int	err;

	err = pthread_create(&world_data->monitor_thread_id, NULL,
			monitor, (void *)world_data);
	if (err != 0)
		return (release_senders(world_data), 1);
	if (coders_create(world_data->coders,
			world_data->args->number_of_coders, world_data) != 0)
	{
		world_stop(world_data);
		release_senders(world_data);
		pthread_join(world_data->monitor_thread_id, NULL);
		return (1);
	}
	if (join_coders(world_data) != 0)
		return (1);
	return (0);
}

int	main(int argc, char **argv)
{
	t_arguments		*arguments;
	t_world_data	*world_data;
	int				err;

	if (parsing_args(argv, argc, &arguments) != 0)
		return (1);
	if (world_data_init(&world_data, arguments) != 0)
		return (free(arguments), 1);
	err = run_simulation(world_data);
	if (err != 0)
		return (free_all(world_data), free(arguments), 1);
	return (free_all(world_data), free(arguments), 0);
}
