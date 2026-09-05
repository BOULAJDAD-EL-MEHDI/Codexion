/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:19:27 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:19:28 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

int	prepare_arrays(t_sim *sim, int count)
{
	sim->dongles = malloc(sizeof(t_dongle) * count);
	if (!sim->dongles)
		return (0);
	sim->coders = malloc(sizeof(t_coder) * count);
	if (!sim->coders)
	{
		free(sim->dongles);
		sim->dongles = NULL;
		return (0);
	}
	init_dongle_scheduling(sim->dongles, count, sim->config->dongle_cooldown);
	return (1);
}

int	init_pair_queues(t_sim *sim, int count)
{
	if (!heap_init(&sim->pair_queue, count))
		return (0);
	if (!heap_init(&sim->pair_scratch, count))
	{
		heap_destroy(&sim->pair_queue);
		return (0);
	}
	return (1);
}

static void	destroy_three_mutexes(t_sim *sim)
{
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->write_mutex);
	pthread_mutex_destroy(&sim->pair_mutex);
}

static int	init_pair_condition(t_sim *sim)
{
	pthread_condattr_t	attr;

	if (pthread_condattr_init(&attr) != 0)
		return (0);
	if (pthread_condattr_setclock(&attr, CLOCK_MONOTONIC) != 0)
	{
		pthread_condattr_destroy(&attr);
		return (0);
	}
	if (pthread_cond_init(&sim->pair_cond, &attr) != 0)
	{
		pthread_condattr_destroy(&attr);
		return (0);
	}
	pthread_condattr_destroy(&attr);
	return (1);
}

int	init_sim_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->write_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->stop_mutex);
		return (0);
	}
	if (pthread_mutex_init(&sim->pair_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->stop_mutex);
		pthread_mutex_destroy(&sim->write_mutex);
		return (0);
	}
	if (!init_pair_condition(sim))
	{
		destroy_three_mutexes(sim);
		return (0);
	}
	return (1);
}

void	destroy_sim_mutexes(t_sim *sim)
{
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->write_mutex);
	pthread_mutex_destroy(&sim->pair_mutex);
	pthread_cond_destroy(&sim->pair_cond);
}
