/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:18:59 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:19:00 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_config	config;
	t_sim		sim;
	int			ok;

	if (!parse_args(argc, argv, &config))
		return (1);
	if (!init_sim(&sim, &config))
	{
		printf("Error: initialization failed\n");
		return (1);
	}
	ok = start_simulation(&sim);
	if (!ok)
		printf("Error: simulation failed to start\n");
	cleanup_sim(&sim);
	return (!ok);
}
