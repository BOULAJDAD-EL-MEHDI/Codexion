/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:47:02 by eboulajd          #+#    #+#             */
/*   Updated: 2026/07/27 00:47:05 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
 
static void	join_coder_threads(t_sim *sim, int count)
{
	int	i;
 
	i = 0;
	while (i < count)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}
 
static int	create_coder_threads(t_sim *sim)
{
	int	i;
	int	count;
 
	i = 0;
	count = sim->config->number_of_coders;
	while (i < count)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
		{
			pthread_mutex_lock(&sim->stop_mutex);
			sim->stop = 1;
			pthread_mutex_unlock(&sim->stop_mutex);
			join_coder_threads(sim, i);
			return (0);
		}
		i++;
	}
	return (1);
}
 
int	start_simulation(t_sim *sim)
{
	if (!create_coder_threads(sim))
		return (0);
	join_coder_threads(sim, sim->config->number_of_coders);
	return (1);
}
 
void	destroy_sim(t_sim *sim)
{
	int	i;
	int	count;
 
	count = sim->config->number_of_coders;
	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->coders[i].action_mutex);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->write_mutex);
	free(sim->coders);
	free(sim->dongles);
}