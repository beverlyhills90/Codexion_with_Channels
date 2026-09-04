#ifndef TEST_MAIN_H
# define TEST_MAIN_H

# include "channels.h"

typedef struct s_worker_args
{
	int			thread_id;
	t_sender	*sender;
	int			rand_num;
}	t_worker_args;

void	*worker(void *arg);

#endif
