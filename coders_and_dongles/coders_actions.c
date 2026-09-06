#include "../codexion.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int	ft_sleep(t_coder *coder, int wait_time_ms)
{
	long long	start_time;
	long long	current_time;

	start_time = get_ms();
	while (safe_world_state_check(coder->world_data) != STOP)
	{
		current_time = get_ms();
		if ((current_time - start_time) >= wait_time_ms)
			return (0);
		usleep(500);
	}
	return (1);
}

int	compile(t_coder *coder)
{
	t_msg	*msg;

	if (safe_world_state_check(coder->world_data) == STOP)
		return (1);
	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
		return (1);
	msg->coder_id = coder->coder_id;
	msg->timestamp = get_ms();
	msg->type = MSG_COMPILING;
	coder->last_compile_timestamp = msg->timestamp;
	if (mpsc_send(coder->log_sender, msg) != 0)
		return (free(msg), 1);
	if (ft_sleep(coder, coder->args->time_to_debug) != 0)
		return (1);
	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
		return (1);
	msg->coder_id = coder->coder_id;
	msg->timestamp = get_ms();
	msg->type = MSG_COMPILE_DONE;
	if (mpsc_send(coder->log_sender, msg) != 0)
		return (free(msg), 1);
	return (0);
}

int	debug(t_coder *coder)
{
	t_msg	*msg;

	if (safe_world_state_check(coder->world_data) == STOP)
		return (1);
	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
		return (1);
	msg->coder_id = coder->coder_id;
	msg->timestamp = get_ms();
	msg->type = MSG_DEBUGGING;
	if (mpsc_send(coder->log_sender, msg) != 0)
		return (free(msg), 1);
	if (ft_sleep(coder, coder->args->time_to_debug) != 0)
		return (1);
	return (0);
}

int	refactoring(t_coder *coder)
{
	t_msg	*msg;

	if (safe_world_state_check(coder->world_data) == STOP)
		return (1);
	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
		return (1);
	msg->coder_id = coder->coder_id;
	msg->timestamp = get_ms();
	msg->type = MSG_REFACTORING;
	if (mpsc_send(coder->log_sender, msg) != 0)
		return (free(msg), 1);
	if (ft_sleep(coder, coder->args->time_to_debug) != 0)
		return (1);
	return (0);
}

void	*coders_routine(void *args)
{
	t_coder	*coder;

	coder = (t_coder *) args;
	while (safe_world_state_check(coder->world_data) == RUNNING)
	{
		if (take_dongle_wrapper(coder) != 0)
			break ;
		if (safe_world_state_check(coder->world_data) == STOP)
			break ;
		if (compile(coder) != 0)
			break ;
		giveup_dongle_wrapper(coder);
		if (debug(coder) != 0)
			break ;
		if (refactoring(coder) != 0)
			break ;
	}
	world_stop(coder->world_data);
	return (NULL);
}
