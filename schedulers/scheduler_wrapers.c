#include "../codexion.h"

void	scheduler_add(t_scheduler scheduler, t_coder *coder, t_dongle *dongle)
{
	t_request	request;

	request.coder = coder;
	request.last_compile_timestamp = coder->last_compile_timestamp;
	if (scheduler == FIFO)
		fifo_scheduler_add(dongle, request);
	else
		edf_scheduler(dongle, request);
}

void	scheduler_del(t_dongle *dongle)
{
	dongle->queue[0] = dongle->queue[1];
	dongle->queue_size--;
}
