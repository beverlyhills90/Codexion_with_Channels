#include "../codexion.h"
#include <pthread.h>

static void	wait_for_dongle(t_dongle *dongle, t_coder *coder)
{
	struct timespec	ts;

	while (safe_world_state_check(coder->world_data) == RUNNING
		&& (dongle->is_occupied || dongle->queue[0].coder != coder
			|| get_ms() < dongle->cooldown))
	{
		if (get_ms() < dongle->cooldown)
		{
			ts.tv_sec = dongle->cooldown / 1000;
			ts.tv_nsec = ((dongle->cooldown) % 1000) * 1000000;
			pthread_cond_timedwait(&dongle->state, &dongle->mutex, &ts);
		}
		else
		{
			pthread_cond_wait(&dongle->state, &dongle->mutex);
		}
	}
}

static int	send_took_dongle(t_coder *coder)
{
	t_msg	*msg;

	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
		return (1);
	msg->coder_id = coder->coder_id;
	msg->timestamp = get_ms();
	msg->type = MSG_TOOK_DONGLE;
	if (mpsc_send(coder->log_sender, msg) != 0)
		return (free(msg), 1);
	return (0);
}

static int	take_dongle(t_dongle *dongle, t_coder *coder)
{
	int	err;

	err = 0;
	pthread_mutex_lock(&dongle->mutex);
	scheduler_add(coder->args->scheduler, coder, dongle);
	wait_for_dongle(dongle, coder);
	if (safe_world_state_check(coder->world_data) == STOP)
	{
		scheduler_del(dongle, coder);
		pthread_mutex_unlock(&dongle->mutex);
		return (1);
	}
	dongle->is_occupied = 1;
	scheduler_del(dongle, coder);
	if (send_took_dongle(coder) != 0)
		err = 1;
	pthread_mutex_unlock(&dongle->mutex);
	return (err);
}

int	take_dongle_wrapper(t_coder *coder)
{
	int	err;

	err = 1;
	if (coder->coder_id % 2 == 0)
	{
		err = take_dongle(coder->right, coder);
		if (err != 0)
			return (1);
		err = take_dongle(coder->left, coder);
		if (err != 0)
			return (1);
	}
	else
	{
		err = take_dongle(coder->left, coder);
		if (err != 0)
			return (1);
		err = take_dongle(coder->right, coder);
		if (err != 0)
			return (1);
	}
	return (0);
}
