#include "../codexion.h"
#include <pthread.h>
#include <stdlib.h>

t_dongle	*dongles_init(unsigned long number)
{
	size_t		i;
	t_dongle	*dongles;

	i = 0;
	dongles = ft_calloc(number, sizeof(t_dongle));
	if (!dongles)
		return (NULL);
	while (i < number)
	{
		dongles[i].cooldown = 0;
		dongles[i].is_occupied = 0;
		if (pthread_cond_init(&dongles[i].state, NULL) != 0)
			return (free_dongles(&dongles, i),NULL);
		if (pthread_mutex_init(&dongles[i].mutex, NULL) != 0)
			return (pthread_cond_destroy(&dongles[i].state),free_dongles(&dongles, i),NULL);
		dongles[i].queue_size = 0;
		i++;
	}
	return (dongles);
}
