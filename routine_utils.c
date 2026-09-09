/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:20:13 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:20:14 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
