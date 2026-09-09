/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:20:38 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:20:39 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <unistd.h>

int	sim_is_stopped(t_sim *sim)
{
	int	stopped;

	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stopped);
}

void	sim_sleep(t_sim *sim, int duration_ms)
{
	long	target;

	if (duration_ms <= 0)
		return ;
	target = get_time_ms() + duration_ms;
	while (get_time_ms() < target && !sim_is_stopped(sim))
		usleep(1000);
}

void	destroy_sim_mutexes(t_sim *sim)
{
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->write_mutex);
	pthread_mutex_destroy(&sim->pair_mutex);
	pthread_cond_destroy(&sim->pair_cond);
}

void	store_config(t_config *config, long *values, char *scheduler)
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

void	join_coder_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->coders_started)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}
