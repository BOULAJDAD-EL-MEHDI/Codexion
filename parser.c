/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 15:08:04 by eboulajd          #+#    #+#             */
/*   Updated: 2026/07/26 22:44:36 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


// static int	is_spositive_number(char *str)
// {
// 	int	i;
//     long res;

// 	i = 0;
//     res = 0;
// 	if (!str[0])
// 		return (0);
//     if (str[0] == '+' || str[0] == '-' )
//         i++;
//     if (!str[i] || str[i] == '+' || str[i] == '-')
//         return (0);
//     if (str[i] == '0')
//         return 0;
// 	while (str[i])
// 	{
// 		if (str[i] < '0' || str[i] > '9')
// 			return (0);
//         res = res * 10  + (str[i] - '0');
//         if (res > 2147483647)
//             return (0);
// 		i++;
// 	}
// 	return (1);
// }

// static int	is_positive_number(char *str)
// {
// 	int	i;
//     long res;

// 	i = 0;
//     res = 0;
// 	if (!str[0])
// 		return (0);
//     if (str[0] == '+' || str[0] == '-' )
//         i++;
//     if (!str[i] || str[i] < '0' || str[i] > '9')
//         return (0);
// 	while (str[i])
// 	{
// 		if (str[i] < '0' || str[i] > '9')
// 			return (0);
//         res = res * 10  + (str[i] - '0');
//         if (res > 2147483647)
//             return (0);
// 		i++;
// 	}
// 	return (1);
// }

long    handle_negative(char *str)
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

long    handle_positive(char *str, long *buffer)
{
    if (!*str)
    {
        printf("Error: Invalid input '+'\n");
        return (-1);
    }
    while (*str)
    {
        if (*str >= '0' && *str <= '9')
        {
            *buffer = (*buffer * 10) + (*str - '0');
            if (*buffer > 0x7fffffff)
            {
                printf("Error: Buffer overflow\n");
                return (-1);
            }
            str++;
        }
        else
        {
            printf("Error: Invalid character\n");
            return (-1);
        }
    }
    return (*buffer);
}

long    custom_atoi(char *str)
{
    long    result;

    result = 0;
    if (*str == '-')
        return (handle_negative(str));
    else if (*str == '+')
        str++;
    return (handle_positive(str, &result));
}

int	parse_args(int argc, char **argv, t_config *config)
{
	if (argc != 9)
	{
		printf("Error !\n");
		return (0);
	}

	if (custom_atoi(argv[1]) == -1 || custom_atoi(argv[2]) == -1
		|| custom_atoi(argv[3]) == -1 || custom_atoi(argv[4]) == -1
		|| custom_atoi(argv[5]) == -1 || custom_atoi(argv[6]) == -1
		|| custom_atoi(argv[7]) == -1)
		return 0;
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (printf("Error: scheduler must be fifo or edf\n"), 0);
	config->number_of_coders = custom_atoi(argv[1]);
	config->time_to_burnout = custom_atoi(argv[2]);
	config->time_to_compile = custom_atoi(argv[3]);
	config->time_to_debug = custom_atoi(argv[4]);
	config->time_to_refactor = custom_atoi(argv[5]);
	config->number_of_compiles_required = custom_atoi(argv[6]);
	config->dongle_cooldown = custom_atoi(argv[7]);
	config->scheduler = argv[8];
	return (1);
}