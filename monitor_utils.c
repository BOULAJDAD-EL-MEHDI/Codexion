/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by eboulajd          #+#    #+#             */
/*   Updated: 2026/07/27 00:00:00 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	coder_burned_out(t_coder *coder, long now)
{
	long	start;
	int		done;
	int		required;
	int		limit;

	pthread_mutex_lock(&coder->action_mutex);
	start = coder->last_compile_start;
	done = coder->compiles_done;
	pthread_mutex_unlock(&coder->action_mutex);
	required = coder->sim->config->number_of_compiles_required;
	limit = coder->sim->config->time_to_burnout;
	if (done >= required)
		return (0);
	return (now - start >= limit);
}

static int	stop_and_log_burnout(t_sim *sim, t_coder *coder, long now)
{
	int		already_stopped;
	long	elapsed;

	pthread_mutex_lock(&sim->write_mutex);
	pthread_mutex_lock(&sim->stop_mutex);
	already_stopped = sim->stop;
	if (!already_stopped)
		sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
	if (!already_stopped)
	{
		elapsed = now - sim->start_time;
		printf("%ld %d burned out\n", elapsed, coder_public_id(coder));
	}
	pthread_mutex_unlock(&sim->write_mutex);
	return (!already_stopped);
}

int	report_burnout(t_sim *sim, t_coder *coder, long now)
{
	int	expired;
	int	logged;

	pthread_mutex_lock(&coder->action_mutex);
	expired = (coder->compiles_done
		< sim->config->number_of_compiles_required
		&& now - coder->last_compile_start
		>= sim->config->time_to_burnout);
	if (!expired)
	{
		pthread_mutex_unlock(&coder->action_mutex);
		return (0);
	}
	logged = stop_and_log_burnout(sim, coder, now);
	pthread_mutex_unlock(&coder->action_mutex);
	return (logged);
}

int	check_completion(t_sim *sim)
{
	int	i;
	int	count;
	int	required;

	i = 0;
	count = sim->config->number_of_coders;
	required = sim->config->number_of_compiles_required;
	while (i < count)
	{
		pthread_mutex_lock(&sim->coders[i].action_mutex);
		if (sim->coders[i].compiles_done < required)
		{
			pthread_mutex_unlock(&sim->coders[i].action_mutex);
			return (0);
		}
		pthread_mutex_unlock(&sim->coders[i].action_mutex);
		i++;
	}
	return (1);
}

void	stop_simulation(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
	pthread_mutex_lock(&sim->pair_mutex);
	pthread_cond_broadcast(&sim->pair_cond);
	pthread_mutex_unlock(&sim->pair_mutex);
}
