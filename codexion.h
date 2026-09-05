#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>
# include "C-Channels/channels.h"
# include "C-Channels/mpsc/cmpsc.h"

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef enum e_msg_type
{
	MSG_TOOK_DONGLE,
	MSG_COMPILING,
	MSG_DEBUGGING,
	MSG_REFACTORING,
	MSG_COMPILE_DONE
}	t_msg_type;

typedef enum e_check_status
{
	RS_OK,
	RS_DONE,
	RS_BURNEDOUT
}	t_check_status;

typedef enum e_running
{
	RUNNING,
	STOP
}	t_running;

typedef struct s_world_data	t_world_data;
typedef struct s_coder		t_coder;

typedef struct s_arguments
{
	size_t		number_of_coders;
	size_t		time_to_burnout;
	size_t		time_to_compile;
	size_t		time_to_debug;
	size_t		time_to_refactor;
	size_t		number_of_compiles_required;
	size_t		dongle_cool_down;
	t_scheduler	scheduler;
}	t_arguments;

typedef struct s_request
{
	long long	last_compile_timestamp;
	t_coder		*coder;
}	t_request;

typedef struct s_dongle
{
	int				dongle_id;
	pthread_cond_t	state;
	pthread_mutex_t	mutex;
	int				is_occupied;
	long long		cooldown;
	t_request		queue[2];
	int				queue_size;
}	t_dongle;

typedef struct s_coder
{
	t_sender		*log_sender;
	unsigned int	coder_id;
	long long		last_compile_timestamp;
	t_dongle		*left;
	t_dongle		*right;
	pthread_t		thread_id;
	t_arguments		*args;
	t_world_data	*world_data;
}	t_coder;

typedef struct s_world_data
{
	t_receiver		*log_rcv;
	t_sender		*log_sender_original;
	pthread_mutex_t	world_mutex;
	t_running		is_running;
	pthread_t		monitor_thread_id;
	t_coder			*coders;
	t_dongle		*dongles;
	long long		*last_compile_time_arr;
	unsigned int	*compilations_done;
	t_arguments		*args;
	long long		time_of_start;
}	t_world_data;

typedef struct s_msg
{
	unsigned int	coder_id;
	t_msg_type		type;
	long long		timestamp;
}	t_msg;

typedef struct s_check_result
{
	t_check_status	status;
	long long		time;
	unsigned int	burned_coder_id;
}	t_check_result;

int				parsing_args(char **argv, int argc, t_arguments **arguments);
t_dongle		*dongles_init(unsigned long number);
t_coder			*coders_init(t_arguments *args, t_dongle *dongles,
					t_world_data *world_data);
int				world_data_init(t_world_data **world_data, t_arguments *args);
int				mutexes_init(t_world_data **world_data, t_arguments *args);
int				ch_init(t_world_data **world_data);
int				coders_create(t_coder *coders, size_t num,
					t_world_data *world_data);

int				take_dongle_wrapper(t_coder *coder);
void			giveup_dongle_wrapper(t_coder *coder);
void			*coders_routine(void *args);

void			scheduler_add(t_scheduler scheduler, t_coder *coder,
					t_dongle *dongle);
void			fifo_scheduler_add(t_dongle *dongle, t_request request);
void			edf_scheduler(t_dongle *dongle, t_request request);
void			scheduler_del(t_dongle *dongle);

long long		get_ms(void);
void			free_dongles(t_dongle **dongles, unsigned int num);
void			free_coders(t_coder *coders, unsigned int num);
void			free_all(t_world_data *world_data);
t_running		safe_world_state_check(t_world_data *world_data);
void			world_stop(t_world_data *world_data);
void			*monitor(void *arg);
void			print_log(t_msg *msg, t_world_data *world_data);
void			set_compile(t_world_data *world_data, t_msg *msg);
t_check_result	check_burn_out(t_world_data *world_data,
					long long time_of_start);

#endif
