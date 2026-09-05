/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:20:29 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:20:31 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

static void	join_coder_threads(t_sim *sim);

static void	reset_start_time(t_sim *sim)
{
	int		i;
	long	start;

	start = get_time_ms();
	sim->start_time = start;
	i = 0;
	while (i < sim->config->number_of_coders)
	{
		pthread_mutex_lock(&sim->coders[i].action_mutex);
		sim->coders[i].last_compile_start = start;
		pthread_mutex_unlock(&sim->coders[i].action_mutex);
		i++;
	}
}

static int	create_coder_threads(t_sim *sim)
{
	int	count;

	count = sim->config->number_of_coders;
	sim->coders_started = 0;
	while (sim->coders_started < count)
	{
		if (pthread_create(&sim->coders[sim->coders_started].thread, NULL,
				coder_routine, &sim->coders[sim->coders_started]) != 0)
			return (0);
		sim->coders_started++;
	}
	return (1);
}

int	start_simulation(t_sim *sim)
{
	if (!create_coder_threads(sim))
	{
		stop_simulation(sim);
		return (0);
	}
	reset_start_time(sim);
	pthread_mutex_lock(&sim->pair_mutex);
	sim->pair_sequence = sim->config->number_of_coders;
	sim->start_ready = 1;
	pthread_cond_broadcast(&sim->pair_cond);
	pthread_mutex_unlock(&sim->pair_mutex);
	sim->monitor_started = create_monitor_thread(sim);
	return (sim->monitor_started);
}

static void	free_sim_resources(t_sim *sim)
{
	int	i;

	if (sim->coders)
	{
		i = 0;
		while (i < sim->config->number_of_coders)
		{
			pthread_mutex_destroy(&sim->coders[i].action_mutex);
			i++;
		}
		free(sim->coders);
		sim->coders = NULL;
	}
	if (sim->dongles)
	{
		free(sim->dongles);
		sim->dongles = NULL;
	}
	heap_destroy(&sim->pair_queue);
	heap_destroy(&sim->pair_scratch);
}

void	cleanup_sim(t_sim *sim)
{
	if (!sim)
		return ;
	if (sim->monitor_started)
		pthread_join(sim->monitor_thread, NULL);
	join_coder_threads(sim);
	free_sim_resources(sim);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->write_mutex);
	pthread_mutex_destroy(&sim->pair_mutex);
	pthread_cond_destroy(&sim->pair_cond);
}

static void	join_coder_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->coders_started)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}
