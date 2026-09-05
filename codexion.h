#ifndef CODEXION_H
#define CODEXION_H
# include <pthread.h>
#include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>
# include "C-Channels/channels.h"
#include "C-Channels/mpsc/cmpsc.h"

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef enum e_msgType{
    MSG_TOOK_DONGLE, MSG_COMPILING, MSG_DEBUGGING,
    MSG_REFACTORING, MSG_COMPILE_DONE
} t_msgType;


typedef enum e_running
{
	RUNNING,
	STOP
}	t_running;

typedef struct s_worldData t_worldData;
typedef struct s_coder t_coder ;

typedef struct s_argumenst
{
	size_t		number_of_coders;
	size_t		time_to_burnout;
	size_t		time_to_compile;
	size_t		time_to_debug;
	size_t		time_to_refactor;
	size_t		number_of_compiles_required;
	size_t		dongle_cool_down;
	t_scheduler scheduler;
}	t_argumnets;


typedef struct s_request {
    long long lastComplieTimestomp;
    t_coder *coder;
} t_request ;

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
    t_sender        *log_sender;
    unsigned int    coder_id;
    long long       lastComplieTimestomp;
    t_dongle        *left;
	t_dongle        *right;
    pthread_t       thread_id;
    t_argumnets     *args;
    t_worldData     *world_data;
    
} t_coder;


typedef struct s_worldData {
    t_receiver *log_rcv;
    t_sender *log_sender_oiginal;
    pthread_mutex_t	world_mutex;
    pthread_mutex_t	output_mutex;
    t_running        is_running;
    pthread_t       monitor_thread_id;
    t_coder			*coders;
    t_dongle        *dongles;
    long long             *lastComplieTimeArr;
    unsigned int    *compilationsDone;
   	t_argumnets		*args;
    long long		timeOfStart;
    
} t_worldData;

typedef struct s_msg 
{
    unsigned int coderId;
    t_msgType type;
    long long timestomp;
}t_msg ;

int	    parsing_args(char **argv, int argc, t_argumnets **arguments);
t_dongle	*dongles_init(unsigned long number);
t_coder	*coders_init(t_argumnets *args, t_dongle *dongles,
		t_worldData *world_data);
int	world_data_init(t_worldData **worldData, t_argumnets *args);

int	coders_create(t_coder *coders, size_t num, t_worldData *world_data);

//coders_and_dongles actions 
void	take_dongle_wraper(t_coder *coder);
void	giveup_dongle_wraper(t_coder *coder);
void *coders_routine(void *args);

//schedulers
void	scheduler_add(t_scheduler scheduler, t_coder *coder, t_dongle *dongle);
void	fifo_scheduler_add(t_dongle *dongle, t_request request);
void	edf_scheduler(t_dongle *dongle, t_request request);
void	scheduler_del(t_dongle *dongle);

//helpers
long long	get_ms(void);
void free_dongles(t_dongle *dongles,size_t nb);
void free_coders(t_coder *coders);
int	world_data_alocation(t_worldData **worldData, t_argumnets *args);
t_running	safeWorldStateCheck(t_worldData *worldData);
void    worldStop(t_worldData *wordData);

//monitor
void *monitor(void *arg);

#endif