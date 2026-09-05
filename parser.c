/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:20:04 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:20:05 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

static long	handle_negative(char *str)
{
	str++;
	if (!*str)
	{
		printf("Error: Invalid input '-'\n");
		return (-1);
	}
	while (*str)
	{
		if (*str != '0')
		{
			printf("Error: Negative numbers\n");
			return (-1);
		}
		str++;
	}
	return (0);
}

static long	handle_positive(char *str, long *buffer)
{
	if (!*str)
	{
		printf("Error: Invalid input '+'\n");
		return (-1);
	}
	while (*str)
	{
		if (*str < '0' || *str > '9')
		{
			printf("Error: Invalid character\n");
			return (-1);
		}
		*buffer = (*buffer * 10) + (*str - '0');
		if (*buffer > 0x7fffffff)
		{
			printf("Error: Buffer overflow\n");
			return (-1);
		}
		str++;
	}
	return (*buffer);
}

long	custom_atoi(char *str)
{
	long	result;

	result = 0;
	if (*str == '-')
		return (handle_negative(str));
	else if (*str == '+')
		str++;
	return (handle_positive(str, &result));
}

static void	store_config(t_config *config, long *values, char *scheduler)
{
	config->number_of_coders = values[0];
	config->time_to_burnout = values[1];
	config->time_to_compile = values[2];
	config->time_to_debug = values[3];
	config->time_to_refactor = values[4];
	config->number_of_compiles_required = values[5];
	config->dongle_cooldown = values[6];
	config->scheduler = scheduler;
}

static int	parse_numbers(char **argv, long *values)
{
	int	i;

	i = 0;
	while (i < 7)
	{
		values[i] = custom_atoi(argv[i + 1]);
		if (values[i] == -1)
			return (0);
		i++;
	}
	return (1);
}

int	parse_args(int argc, char **argv, t_config *config)
{
	long	values[7];

	if (argc != 9)
	{
		printf("Error !\n");
		return (0);
	}
	if (!parse_numbers(argv, values))
		return (0);
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (printf("Error: scheduler must be fifo or edf\n"), 0);
	if (values[0] == 0)
		return (printf("Error: number_of_coders must be positive\n"), 0);
	store_config(config, values, argv[8]);
	return (1);
}
