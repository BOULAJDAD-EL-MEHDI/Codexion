/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 14:51:09 by eboulajd          #+#    #+#             */
/*   Updated: 2026/07/27 00:46:42 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
 
int	main(int argc, char **argv)
{
	t_config	config;
	t_sim		sim;
 
	if (!parse_args(argc, argv, &config))
		return (1);
	if (!init_sim(&sim, &config))
	{
		printf("Error: initialization failed\n");
		return (1);
	}
	if (!start_simulation(&sim))
	{
		printf("Error: simulation failed to start\n");
		destroy_sim(&sim);
		return (1);
	}
	destroy_sim(&sim);
	return (0);
}