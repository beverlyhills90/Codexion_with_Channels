#include "../codexion.h"

static void	take_dongle(t_dongle *dongle, t_coder *coder)
{
	struct timespec	ts;
	t_msg *msg;

	pthread_mutex_lock(&dongle->mutex);
	scheduler_add(coder->args->scheduler, coder, dongle);
	while (safeWorldStateCheck(coder->world_data) == RUNNING
		&& (dongle->is_occupied || get_ms() < dongle->cooldown
			|| dongle->queue[0].coder != coder))
	{
		ts.tv_sec = dongle->cooldown / 1000;
		ts.tv_nsec = ((dongle->cooldown) % 1000) * 1000000;
		pthread_cond_timedwait(&dongle->state, &dongle->mutex, &ts);
	}
	if (safeWorldStateCheck(coder->world_data) == STOP)
	{
		scheduler_del(dongle);
		pthread_mutex_unlock(&dongle->mutex);
		return ;
	}
	dongle->is_occupied = 1;
	scheduler_del(dongle);
	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
	    return ;//MB ADD FALBACK
	msg->coderId = coder->coder_id;
	msg->timestomp = get_ms();
	msg->type = MSG_TOOK_DONGLE;
	mpsc_send(coder->log_sender, msg);
	pthread_mutex_unlock(&dongle->mutex);
}

void	take_dongle_wraper(t_coder *coder)
{
	if (coder->coder_id % 2 == 0)
	{
		take_dongle(coder->right, coder);
		take_dongle(coder->left, coder);
	}
	else
	{
		take_dongle(coder->left, coder);
		take_dongle(coder->right, coder);
	}
}


static void	giveup_dongle(t_dongle *dongle, long long cooldown)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->cooldown = get_ms() + cooldown;
	dongle->is_occupied = 0;
	pthread_cond_broadcast(&dongle->state);
	pthread_mutex_unlock(&dongle->mutex);
}

void	giveup_dongle_wraper(t_coder *coder)
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