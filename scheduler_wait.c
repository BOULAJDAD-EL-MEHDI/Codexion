/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_wait.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:20:24 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:20:25 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	next_dongle_ready(t_sim *sim)
{
	long	next;
	int		i;

	next = 0;
	i = 0;
	while (i < sim->config->number_of_coders)
	{
		if (sim->dongles[i].available_at > 0
			&& (next == 0 || sim->dongles[i].available_at < next))
			next = sim->dongles[i].available_at;
		i++;
	}
	return (next);
}

static void	set_wait_limit(struct timespec *limit, long wait_ms)
{
	clock_gettime(CLOCK_REALTIME, limit);
	limit->tv_sec += wait_ms / 1000;
	limit->tv_nsec += (wait_ms % 1000) * 1000000;
	if (limit->tv_nsec >= 1000000000)
	{
		limit->tv_sec++;
		limit->tv_nsec -= 1000000000;
	}
}

static void	wait_for_dongle_timer(t_sim *sim, long ready)
{
	struct timespec	limit;
	long			wait_ms;

	sim->timer_waiting = 1;
	wait_ms = ready - get_time_ms();
	if (wait_ms < 0)
		wait_ms = 0;
	set_wait_limit(&limit, wait_ms);
	pthread_cond_timedwait(&sim->pair_cond, &sim->pair_mutex, &limit);
	sim->timer_waiting = 0;
}

static void	wait_for_scheduler(t_sim *sim)
{
	long	ready;

	ready = next_dongle_ready(sim);
	if (ready == 0 || sim->timer_waiting)
		pthread_cond_wait(&sim->pair_cond, &sim->pair_mutex);
	else
		wait_for_dongle_timer(sim, ready);
}

int	wait_for_grant(t_sim *sim, t_coder *coder, int *granted)
{
	t_request	withdrawn;

	while (!*granted && !sim_is_stopped(sim))
	{
		wait_for_scheduler(sim);
		if (try_grant_pairs(sim) > 0)
			pthread_cond_broadcast(&sim->pair_cond);
	}
	if (*granted)
		return (1);
	heap_remove_coder(&sim->pair_queue, coder->id, &withdrawn);
	return (0);
}
