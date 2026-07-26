/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 14:51:09 by eboulajd          #+#    #+#             */
/*   Updated: 2026/07/26 15:06:57 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_config	config;

	if (!parse_args(argc, argv, &config))
		return (1);

	printf("Number of coders        : %d\n", config.number_of_coders);
	printf("Time to burnout         : %d\n", config.time_to_burnout);
	printf("Time to compile         : %d\n", config.time_to_compile);
	printf("Time to debug           : %d\n", config.time_to_debug);
	printf("Time to refactor        : %d\n", config.time_to_refactor);
	printf("Compiles required       : %d\n", config.number_of_compiles_required);
	printf("Dongle cooldown         : %d\n", config.dongle_cooldown);
	printf("Scheduler               : %s\n", config.scheduler);
	return (0);
}