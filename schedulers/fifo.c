#include "../codexion.h"

void	fifo_scheduler_add(t_dongle *dongle,t_request request)
{
	if (dongle->queue_size == 0)
	    dongle->queue[0] = request;
	else
	    dongle->queue[1] = request;
	dongle->queue_size++;
}