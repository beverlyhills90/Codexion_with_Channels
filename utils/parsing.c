#include "../codexion.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

int	ft_isnum(char c)
{
	if (c >= '0' && c <= '9')
		return (1);
	return (0);
}

int	parse_arg_to_int(char *str, size_t *out_val)
{
	int	res;
	int	d;

	if (!str || *str == '\0')
		return (1);
	res = 0;
	while (*str)
	{
		if (ft_isnum(*str) != 1)
			return (1);
		d = *str - '0';
		if (res > (INT_MAX - d) / 10)
			return (1);
		res = res * 10 + d;
		str++;
	}
	*out_val = res;
	return (0);
}

int	fill_args_struct(char **argv, t_arguments *arguments)
{
	if (parse_arg_to_int(argv[1], &arguments->number_of_coders) != 0
		|| parse_arg_to_int(argv[2], &arguments->time_to_burnout) != 0
		|| parse_arg_to_int(argv[3], &arguments->time_to_compile) != 0
		|| parse_arg_to_int(argv[4], &arguments->time_to_debug) != 0
		|| parse_arg_to_int(argv[5], &arguments->time_to_refactor) != 0
		|| parse_arg_to_int(argv[6],
			&arguments->number_of_compiles_required) != 0
		|| parse_arg_to_int(argv[7], &arguments->dongle_cool_down) != 0)
	{
		return (fprintf(stderr,
				"Error: Invalid numeric argument or overflow\n"), 1);
	}
	if (strcmp(argv[8], "fifo") == 0)
		arguments->scheduler = FIFO;
	else if (strcmp(argv[8], "edf") == 0)
		arguments->scheduler = EDF;
	else
		return (fprintf(stderr, "Error: Scheduler argument is invalid\n"), 1);
	return (0);
}

int	parsing_args(char **argv, int argc, t_arguments **arguments)
{
	if (argc != 9)
		return (fprintf(stderr, "Invalid argument quantity\n"), 1);
	*arguments = ft_calloc(1, sizeof(t_arguments));
	if (!*arguments)
		return (fprintf(stderr, "allocation failed\n"), 1);
	if (fill_args_struct(argv, *arguments) != 0)
	{
		free(*arguments);
		return (1);
	}
	if ((*arguments)->number_of_coders == 0)
		return (printf("Error: number of coders <= 0\n"), 1);
	return (0);
}
