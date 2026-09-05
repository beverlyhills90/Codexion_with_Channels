#include "../codexion.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static int	handle_check(t_world_data *world_data, t_check_result check_res)
{
	if (check_res.status == RS_BURNEDOUT)
	{
		printf("%llu %u burned out\n", check_res.time,
			check_res.burned_coder_id);
		return (1);
	}
	if (check_res.status == RS_DONE)
		return (world_stop(world_data), 1);
	return (0);
}

static void	handle_msg(t_world_data *world_data, t_msg *msg)
{
	if (msg->type != MSG_COMPILE_DONE)
		print_log(msg, world_data);
	if (msg->type == MSG_COMPILING || msg->type == MSG_COMPILE_DONE)
		set_compile(world_data, msg);
	free(msg);
}

void	*monitor(void *arg)
{
	t_world_data	*world_data;
	t_chan_result	res;
	t_msg			*msg;
	t_check_result	check_res;

	world_data = (t_world_data *)arg;
	while (1)
	{
		check_res = check_burn_out(world_data, world_data->time_of_start);
		if (handle_check(world_data, check_res) != 0)
			return (NULL);
		res = mpsc_recv_until(world_data->log_rcv,
				check_res.time + world_data->time_of_start);
		msg = res.data;
		if (res.status == CH_CLOSED)
			break ;
		if (res.status == CH_TIMEOUT)
			continue ;
		handle_msg(world_data, msg);
	}
	return (NULL);
}
