#include "codexion.h"


int	join_coders(t_worldData *world_data)
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
	if (pthread_join(world_data->monitor_thread_id, NULL) != 0)
		has_error = 1;
	return (has_error);
}


int	join_all(t_worldData *world_data)
{
	int	err;

	err = pthread_create(&world_data->monitor_thread_id, NULL,
			monitor, (void *)world_data);
	if (err != 0)
		return (1);
	if (coders_create(world_data->coders,
			world_data->args->number_of_coders, world_data) != 0)
	{
		worldStop(world_data);
		pthread_join(world_data->monitor_thread_id, NULL);
		return (1);
	}
	if (join_coders(world_data) != 0)
		return (1);
	return (0);
}


int main(int argc,char **argv)
{
    t_argumnets		*arguments;
    t_worldData     *worldData;
    int				err;
    
    if (parsing_args(argv, argc, &arguments) != 0)
		return (1);
    if (world_data_init(&worldData,  arguments) != 0)
        return (free(arguments), 1);
    err = join_all(worldData);
	if (err != 0)
		return (1); //TODO clean up

}