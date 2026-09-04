#include "test_main.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void	*worker(void *arg)
{
	t_worker_args	*args;

	args = arg;
	usleep(args->rand_num);
	mpsc_send(args->sender, &args->thread_id);
	mpsc_sender_drop(args->sender);
	return (NULL);
}

static void	drain(t_mpsc *mpsc)
{
	t_chan_result	res;

	while (1)
	{
		res = mpsc_recv(mpsc->receiver);
		if (res.status == CH_CLOSED)
			break ;
		printf("HI from : %i \n", *(int *)res.data);
	}
}

int	main(void)
{
	t_mpsc			*mpsc;
	pthread_t		threads[5];
	t_worker_args	*args;
	int				i;

	srand(time(NULL));
	mpsc = mpsc_new();
	if (!mpsc)
		return (1);
	i = 0;
	while (i < 5)
	{
		args = malloc(sizeof(t_worker_args));
		args->sender = mpsc_sender_clone(mpsc->sender);
		args->thread_id = i;
		args->rand_num = rand() % 100 + i;
		printf("Random number: %d\n", args->rand_num);
		pthread_create(&threads[i], NULL, worker, args);
		pthread_join(threads[i], NULL);
		i++;
	}
	mpsc_sender_drop(mpsc->sender);
	drain(mpsc);
	mpsc_free(mpsc);
	return (0);
}
