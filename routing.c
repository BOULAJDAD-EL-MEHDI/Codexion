/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:47:20 by eboulajd          #+#    #+#             */
/*   Updated: 2026/07/27 00:50:37 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
 
static void	log_action(t_sim *sim, int id, char *msg)
{
	long	elapsed;
	int		stopped;
 
	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	if (stopped)
		return ;
	elapsed = get_time_ms() - sim->start_time;
	pthread_mutex_lock(&sim->write_mutex);
	printf("%ld %d %s\n", elapsed, id, msg);
	pthread_mutex_unlock(&sim->write_mutex);
}
 
static void	coder_sleep(t_sim *sim, int duration_ms)
{
	long	target;
	int		stopped;
 
	target = get_time_ms() + duration_ms;
	stopped = 0;
	while (get_time_ms() < target && !stopped)
	{
		usleep(1000);
		pthread_mutex_lock(&sim->stop_mutex);
		stopped = sim->stop;
		pthread_mutex_unlock(&sim->stop_mutex);
	}
}
 
static void	take_dongles(t_coder *coder, t_sim *sim)
{
	if (coder->id % 2 == 0)
	{
		pthread_mutex_lock(&coder->left_dongle->mutex);
		pthread_mutex_lock(&coder->right_dongle->mutex);
	}
	else
	{
		pthread_mutex_lock(&coder->right_dongle->mutex);
		pthread_mutex_lock(&coder->left_dongle->mutex);
	}
	log_action(sim, coder->id, "has taken a dongle");
}
 
void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;
 
	coder = (t_coder *)arg;
	sim = coder->sim;
	while (coder->compiles_done < sim->config->number_of_compiles_required
		&& !sim->stop)
	{
		take_dongles(coder, sim);
		log_action(sim, coder->id, "is compiling");
		coder_sleep(sim, sim->config->time_to_compile);
		pthread_mutex_unlock(&coder->left_dongle->mutex);
		pthread_mutex_unlock(&coder->right_dongle->mutex);
		pthread_mutex_lock(&coder->action_mutex);
		coder->compiles_done++;
		coder->last_action_time = get_time_ms();
		pthread_mutex_unlock(&coder->action_mutex);
		log_action(sim, coder->id, "is debugging");
		coder_sleep(sim, sim->config->time_to_debug);
		log_action(sim, coder->id, "is refactoring");
		coder_sleep(sim, sim->config->time_to_refactor);
	}
	return (NULL);
}