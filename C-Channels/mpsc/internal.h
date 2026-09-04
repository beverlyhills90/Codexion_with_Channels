#ifndef INTERNAL_H
# define INTERNAL_H

# include "cmpsc.h"

/*
** Private layouts. Included only by the library sources, never by
** channels.h, so t_sender and t_receiver stay opaque to library users.
*/

struct s_sender
{
	t_channel	*channel_q;
};

struct s_receiver
{
	t_channel	*channel_q;
};

#endif
