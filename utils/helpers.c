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



t_running	safeWorldStateCheck(t_worldData *worldData)
{
	t_running	res;

	pthread_mutex_lock(&worldData->world_mutex);
	if (worldData->is_running == RUNNING)
		res = RUNNING;
	else
		res = STOP;
	pthread_mutex_unlock(&worldData->world_mutex);
	return (res);
}

void worldStop(t_worldData *wordData)
{
    size_t i;
    pthread_mutex_lock(&wordData->world_mutex);
	wordData->is_running = STOP;
	pthread_mutex_unlock(&wordData->world_mutex);
	while (i < wordData->args->number_of_coders)
	{
        pthread_mutex_lock(&wordData->dongles[i].mutex);
		pthread_cond_broadcast(&wordData->dongles[i].state);
		pthread_mutex_unlock(&wordData->dongles[i].mutex);
		i++;
	}
}

