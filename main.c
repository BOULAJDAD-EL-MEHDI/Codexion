/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: voldemort <voldemort@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 17:28:56 by voldemort         #+#    #+#             */
/*   Updated: 2026/07/28 17:32:06 by voldemort        ###   ########.fr       */
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
