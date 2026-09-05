#include "../codexion.h"

void	fill_args_struct(char **argv, t_arguments *arguments)
{
	arguments->number_of_coders = atoi(argv[1]);
	arguments->time_to_burnout = atoi(argv[2]);
	arguments->time_to_compile = atoi(argv[3]);
	arguments->time_to_debug = atoi(argv[4]);
	arguments->time_to_refactor = atoi(argv[5]);
	arguments->number_of_compiles_required = atoi(argv[6]);
	arguments->dongle_cool_down = atoi(argv[7]);
	if (strcmp(argv[8], "fifo") == 0)
		arguments->scheduler = FIFO;
	else
		arguments->scheduler = EDF;
}

int	ft_isnum(char c)
{
	if (c >= '0' && c <= '9')
		return (1);
	return (0);
}

// 0 - invalid input 1 - valid input
int	validate_args(char **argv, int argc)
{
	int	i;
	int	j;

	i = 1;
	if (argc != 9)
		return (fprintf(stderr, "Invalid argument quantity\n"), 1);
	while (i <= 7)
	{
		j = 0;
		while (argv[i][j])
		{
			if (ft_isnum(argv[i][j]) != 1)
			{
				return (fprintf(stderr, "Argument %s is invalid\n", argv[i]),
					1);
			}
			j++;
		}
		i++;
	}
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (fprintf(stderr, "Scheduler argument is invalid\n"), 1);
	return (0);
}

int	parsing_args(char **argv, int argc, t_arguments **arguments)
{
	if (validate_args(argv, argc) != 0)
		return (1);
	*arguments = ft_calloc(1, sizeof(t_arguments));
	if (!*arguments)
		return (fprintf(stderr, "allocation failed"), 1);
	fill_args_struct(argv, *arguments);
	return (0);
}
