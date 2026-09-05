#include "../codexion.h"

static void	giveup_dongle(t_dongle *dongle, long long cooldown)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->cooldown = get_ms() + cooldown;
	dongle->is_occupied = 0;
	pthread_cond_broadcast(&dongle->state);
	pthread_mutex_unlock(&dongle->mutex);
}

void	giveup_dongle_wrapper(t_coder *coder)
{
	if (coder->coder_id % 2 == 0)
	{
		giveup_dongle(coder->left, coder->args->dongle_cool_down);
		giveup_dongle(coder->right, coder->args->dongle_cool_down);
	}
	else
	{
		giveup_dongle(coder->right, coder->args->dongle_cool_down);
		giveup_dongle(coder->left, coder->args->dongle_cool_down);
	}
}
