/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:19:33 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:19:34 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"
#include <time.h>

long	get_time_ms(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000));
}

static int	init_one_coder(t_sim *sim, int index, int count)
{
	t_coder	*coder;

	coder = &sim->coders[index];
	coder->id = index;
	coder->compiles_done = 0;
	coder->last_compile_start = sim->start_time;
	coder->left_dongle = &sim->dongles[index];
	coder->right_dongle = &sim->dongles[(index + 1) % count];
	coder->sim = sim;
	return (pthread_mutex_init(&coder->action_mutex, NULL) == 0);
}

static int	init_coders(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (!init_one_coder(sim, i, count))
		{
			while (i > 0)
			{
				i--;
				pthread_mutex_destroy(&sim->coders[i].action_mutex);
			}
			return (0);
		}
		i++;
	}
	return (1);
}

static int	init_resources(t_sim *sim, int count)
{
	if (!init_pair_queues(sim, count))
		return (0);
	if (!prepare_arrays(sim, count))
	{
		heap_destroy(&sim->pair_queue);
		heap_destroy(&sim->pair_scratch);
		return (0);
	}
	if (!init_coders(sim, count))
	{
		free(sim->dongles);
		sim->dongles = NULL;
		free(sim->coders);
		sim->coders = NULL;
		heap_destroy(&sim->pair_queue);
		heap_destroy(&sim->pair_scratch);
		return (0);
	}
	return (1);
}

int	init_sim(t_sim *sim, t_config *config)
{
	sim->config = config;
	sim->stop = 0;
	sim->coders_done = 0;
	sim->coders_started = 0;
	sim->monitor_started = 0;
	sim->start_ready = 0;
	sim->initial_requests = 0;
	sim->timer_waiting = 0;
	sim->dongles = NULL;
	sim->coders = NULL;
	sim->pair_sequence = 0;
	sim->start_time = get_time_ms();
	if (!init_sim_mutexes(sim))
		return (0);
	if (!init_resources(sim, config->number_of_coders))
	{
		destroy_sim_mutexes(sim);
		return (0);
	}
	return (1);
}
