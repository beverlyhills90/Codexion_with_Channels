CC = cc
CFLAGS = -g -Wall -Wextra -Werror -pthread
DEPFLAGS = -MMD -MP
AR = ar
ARFLAGS = rcs

SRCS = helpers.c \
       mpsc_pub_funcs.c \
       mpsc/channel.c \
       mpsc/receive.c \
       mpsc/send.c

OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d) test_main.d

STATIC_LIB = libmpsc.a
TEST_BIN = test_app

.PHONY: all clean fclean re test norm

all: $(STATIC_LIB)

$(STATIC_LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): test_main.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

norm:
	norminette helpers.c helpers.h channels.h mpsc/ \
		mpsc_pub_funcs.c test_main.c

clean:
	rm -f $(OBJS) $(DEPS) test_main.o

fclean: clean
	rm -f $(STATIC_LIB) $(TEST_BIN)

re: fclean all

-include $(DEPS)
