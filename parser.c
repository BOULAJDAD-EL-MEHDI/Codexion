/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 15:08:04 by eboulajd          #+#    #+#             */
/*   Updated: 2026/07/26 15:54:36 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_positive_number(char *str)
{
	int	i;
    long res;

	i = 0;
    res = 0;
	if (!str[0])
		return (0);
    if (str[0] == '+' && (str[1] >= '0' && str[1] <= '9'))
        i++;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
        res = res * 10  + (str[i] - '0');
        if (res > 2147483647)
            return (0);
		i++;
	}
	return (1);
}

int	parse_args(int argc, char **argv, t_config *config)
{
	if (argc != 9)
	{
		printf("Error !\n");
		return (0);
	}
	if (!is_positive_number(argv[1]) || !is_positive_number(argv[2])
		|| !is_positive_number(argv[3]) || !is_positive_number(argv[4])
		|| !is_positive_number(argv[5]) || !is_positive_number(argv[6])
		|| !is_positive_number(argv[7]))
		return (printf("Error: invalid numeric argument\n"), 0);
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (printf("Error: scheduler must be fifo or edf\n"), 0);
	config->number_of_coders = atoi(argv[1]);
	config->time_to_burnout = atoi(argv[2]);
	config->time_to_compile = atoi(argv[3]);
	config->time_to_debug = atoi(argv[4]);
	config->time_to_refactor = atoi(argv[5]);
	config->number_of_compiles_required = atoi(argv[6]);
	config->dongle_cooldown = atoi(argv[7]);
	config->scheduler = argv[8];
	return (1);
}