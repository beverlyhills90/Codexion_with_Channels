NAME = codexion

CC = cc
CFLAGS = -g #-Wall -Wextra -Werror
 
SRCS = main.c \
	world_data.c \
	utils/parsing.c \
	utils/helpers.c \
	utils/monitor.c \
	utils/freeall.c \
	coders_and_dongles/coders_init.c \
	coders_and_dongles/coders_actions.c \
	coders_and_dongles/dongles_init.c \
	coders_and_dongles/dongles.c \
	schedulers/fifo.c \
	schedulers/edf.c \
	schedulers/scheduler_wrapers.c \
	C-Channels/helpers.c \
	C-Channels/mpsc_pub_funcs.c \
	C-Channels/mpsc/channel.c \
	C-Channels/mpsc/send.c \
	C-Channels/mpsc/receive.c

OBJS = $(SRCS:.c=.o)


HEADER = codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) -pthread


%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

fun:
	@make re CC=x86_64-linux-gnu-gcc CFLAGS="$(filter-out -m64,$(CFLAGS))"

re: fclean all


.PHONY: all clean fclean re