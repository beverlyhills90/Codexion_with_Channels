#include "../codexion.h"
#include <string.h>


void	compile(t_coder *coder)
{
	long long	log_time;
	t_msg *msg;

	
	if (safeWorldStateCheck(coder->world_data) == STOP)
		return ;
	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
	    return; //TODO MB add falback
	msg->coderId = coder->coder_id;
	msg->timestomp = get_ms();
	msg->type = MSG_COMPILING;
	coder->lastComplieTimestomp = msg->timestomp;
	mpsc_send(coder->log_sender,msg);
	usleep(coder->args->time_to_compile * 1000);
	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
	    return ; //TODO MB add falback
	msg->coderId = coder->coder_id;
	msg->timestomp = get_ms();
	msg->type = MSG_COMPILE_DONE;
	mpsc_send(coder->log_sender,msg);
}

void	debug(t_coder *coder)
{
	long long	log_time;
	t_msg *msg;
	
	if (safeWorldStateCheck(coder->world_data) == STOP)
		return ;
	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
	    return; //TODO MB add falback
	msg->coderId = coder->coder_id;
	msg->timestomp = get_ms();
	msg->type = MSG_DEBUGGING;
	mpsc_send(coder->log_sender,msg);
	usleep(coder->args->time_to_debug * 1000);
	
}

void	refractoring(t_coder *coder)
{
	long long	log_time;
	t_msg *msg;
		
	if (safeWorldStateCheck(coder->world_data) == STOP)
		return ;
	msg = ft_calloc(1, sizeof(t_msg));
	if (!msg)
	    return; //TODO MB add falback
	msg->coderId = coder->coder_id;
	msg->timestomp = get_ms();
	msg->type = MSG_REFACTORING;
	mpsc_send(coder->log_sender,msg);
	usleep(coder->args->time_to_refactor * 1000);
}


void *coders_routine(void *args)
{
    t_coder *coder;

    coder = (t_coder *) args;
    while (safeWorldStateCheck(coder->world_data) == RUNNING) 
    {

        take_dongle_wraper(coder);
		if (safeWorldStateCheck(coder->world_data) == STOP)
		{
			giveup_dongle_wraper(coder);
			break;
		}
        compile(coder);
        giveup_dongle_wraper(coder);
        debug(coder);
        refractoring(coder);
    }
    mpsc_sender_drop(coder->log_sender);
    return (NULL);
}