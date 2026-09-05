#include "../codexion.h"
#include <stddef.h>

long long	get_ms(void)
{
	struct timeval	tv;
	long long		milliseconds;

	gettimeofday(&tv, NULL);
	milliseconds = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	return (milliseconds);
}

t_running	safe_world_state_check(t_world_data *world_data)
{
	t_running	res;

	pthread_mutex_lock(&world_data->world_mutex);
	if (world_data->is_running == RUNNING)
		res = RUNNING;
	else
		res = STOP;
	pthread_mutex_unlock(&world_data->world_mutex);
	return (res);
}

void	world_stop(t_world_data *world_data)
{
	size_t	i;

	i = 0;
	pthread_mutex_lock(&world_data->world_mutex);
	world_data->is_running = STOP;
	pthread_mutex_unlock(&world_data->world_mutex);
	while (i < world_data->args->number_of_coders)
	{
		pthread_mutex_lock(&world_data->dongles[i].mutex);
		pthread_cond_broadcast(&world_data->dongles[i].state);
		pthread_mutex_unlock(&world_data->dongles[i].mutex);
		i++;
	}
}
