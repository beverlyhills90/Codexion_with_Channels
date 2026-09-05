#include "../codexion.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void	print_log(t_msg *msg, t_world_data *world_data)
{
	long long	log_time;
	char		*logs[4];

	logs[0] = " has taken a dongle";
	logs[1] = " is compiling";
	logs[2] = " is debugging";
	logs[3] = " is refactoring";
	if (safe_world_state_check(world_data) == STOP)
		return ;
	log_time = msg->timestamp - world_data->time_of_start;
	printf("%llu %i%s\n", log_time, msg->coder_id, logs[msg->type]);
}

void	set_compile(t_world_data *world_data, t_msg *msg)
{
	if (msg->type == MSG_COMPILING)
		world_data->last_compile_time_arr[msg->coder_id - 1]
			= msg->timestamp - world_data->time_of_start;
	else
		world_data->compilations_done[msg->coder_id - 1]++;
}
