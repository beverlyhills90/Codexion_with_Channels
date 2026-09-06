#include "../codexion.h"
#include <stddef.h>
#include <stdlib.h>

static t_check_result	burnout_result(t_world_data *world_data, size_t i)
{
	t_check_result	result;

	world_stop(world_data);
	result.status = RS_BURNEDOUT;
	result.time = world_data->last_compile_time_arr[i]
		+ world_data->args->time_to_burnout;
	result.burned_coder_id = i + 1;
	return (result);
}

static t_check_result	scan_result(t_world_data *world_data, size_t done,
		long long min_time)
{
	t_check_result	result;

	if (done == world_data->args->number_of_coders)
		result.status = RS_DONE;
	else
		result.status = RS_OK;
	result.time = min_time + world_data->args->time_to_burnout;
	result.burned_coder_id = 0;
	return (result);
}

t_check_result	check_burn_out(t_world_data *world_data,
		long long time_of_start)
{
	long long	min_time;
	size_t		i;
	size_t		done;

	i = 0;
	done = 0;
	min_time = world_data->last_compile_time_arr[0];
	while (i < world_data->args->number_of_coders)
	{
		if (world_data->last_compile_time_arr[i] < min_time)
			min_time = world_data->last_compile_time_arr[i];
		if (world_data->last_compile_time_arr[i]
			+ (long long)world_data->args->time_to_burnout
			<= get_ms() - time_of_start)
			return (burnout_result(world_data, i));
		if (world_data->args->number_of_compiles_required
			<= world_data->compilations_done[i])
			done++;
		i++;
	}
	return (scan_result(world_data, done, min_time));
}
