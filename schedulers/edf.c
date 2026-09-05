#include "../codexion.h"

void	edf_scheduler(t_dongle *dongle, t_request request)
{
	t_request	tmp;

	if (dongle->queue_size == 0)
	{
		dongle->queue[0] = request;
	}
	else
	{
		if (dongle->queue[0].last_compile_timestamp
			<= request.last_compile_timestamp)
			dongle->queue[1] = request;
		else
		{
			tmp = dongle->queue[0];
			dongle->queue[0] = request;
			dongle->queue[1] = tmp;
		}
	}
	dongle->queue_size++;
}
