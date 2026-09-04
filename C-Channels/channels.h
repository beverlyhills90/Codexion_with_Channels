#ifndef CHANNELS_H
# define CHANNELS_H

# include "mpsc/cmpsc.h"

typedef struct s_mpsc
{
	t_receiver	*receiver;
	t_sender	*sender;
}	t_mpsc;

t_mpsc	*mpsc_new(void);
void	mpsc_free(t_mpsc *mpsc);

#endif
