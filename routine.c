/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: voldemort <voldemort@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by eboulajd          #+#    #+#             */
/*   Updated: 2026/08/02 15:28:13 by voldemort        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	log_action_at(t_sim *sim, int id, char *msg, long elapsed)
{
	int	stopped;

	pthread_mutex_lock(&sim->write_mutex);
	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	if (!stopped)
		printf("%ld %d %s\n", elapsed, id, msg);
	pthread_mutex_unlock(&sim->write_mutex);
}

static void	log_action(t_sim *sim, int id, char *msg)
{
	log_action_at(sim, id, msg, get_time_ms() - sim->start_time);
}

static int	take_dongles(t_coder *coder, t_sim *sim, long *started)
{
	long	elapsed;

	if (!scheduler_acquire_pair(coder))
		return (0);
	pthread_mutex_lock(&coder->action_mutex);
	*started = coder->last_compile_start;
	pthread_mutex_unlock(&coder->action_mutex);
	elapsed = get_time_ms() - sim->start_time;
	log_action_at(sim, coder_public_id(coder), "has taken a dongle", elapsed);
	log_action_at(sim, coder_public_id(coder), "has taken a dongle", elapsed);
	return (1);
}

static int	run_compile_cycle(t_coder *coder, t_sim *sim)
{
	long	started;

	if (!take_dongles(coder, sim, &started))
		return (0);
	log_action(sim, coder_public_id(coder), "is compiling");
	sim_sleep(sim, started + sim->config->time_to_compile - get_time_ms());
	scheduler_release_pair(coder);
	pthread_mutex_lock(&coder->action_mutex);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->action_mutex);
	if (sim_is_stopped(sim))
		return (0);
	log_action(sim, coder_public_id(coder), "is debugging");
	sim_sleep(sim, started + sim->config->time_to_compile
		+ sim->config->time_to_debug - get_time_ms());
	log_action(sim, coder_public_id(coder), "is refactoring");
	sim_sleep(sim, started + sim->config->time_to_compile
		+ sim->config->time_to_debug + sim->config->time_to_refactor
		- get_time_ms());
	return (1);
}

static void	run_single_coder(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->pair_mutex);
	coder->left_dongle->available = 0;
	pthread_mutex_unlock(&sim->pair_mutex);
	log_action(sim, coder_public_id(coder), "has taken a dongle");
	sim_sleep(sim, sim->config->time_to_burnout + 1);
}

static int	wait_for_start(t_sim *sim)
{
	pthread_mutex_lock(&sim->pair_mutex);
	while (!sim->start_ready && !sim_is_stopped(sim))
		pthread_cond_wait(&sim->pair_cond, &sim->pair_mutex);
	pthread_mutex_unlock(&sim->pair_mutex);
	return (!sim_is_stopped(sim));
}

static int	coder_finished(t_coder *coder)
{
	int	done;

	pthread_mutex_lock(&coder->action_mutex);
	done = (coder->compiles_done
			>= coder->sim->config->number_of_compiles_required);
	pthread_mutex_unlock(&coder->action_mutex);
	return (done);
}

static void	run_coder_cycles(t_coder *coder)
{
	int	done;

	done = 0;
	while (!done && !sim_is_stopped(coder->sim))
	{
		if (!run_compile_cycle(coder, coder->sim))
			break ;
		done = coder_finished(coder);
	}
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	if (!wait_for_start(sim))
		return (NULL);
	if (sim->config->number_of_coders == 1)
	{
		run_single_coder(coder);
		return (NULL);
	}
	run_coder_cycles(coder);
	return (NULL);
}
