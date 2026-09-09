/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_request.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:20:24 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:20:25 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	prepare_request(t_sim *sim, t_coder *coder,
		t_request *request, int *granted)
{
	int			initial;

	pthread_mutex_lock(&coder->action_mutex);
	initial = (coder->compiles_done == 0);
	pthread_mutex_unlock(&coder->action_mutex);
	request->coder_id = coder->id;
	if (initial)
		request->sequence = coder->id + 1;
	else
		request->sequence = ++sim->pair_sequence;
	request->priority = compute_pair_priority(sim, coder);
	request->granted = granted;
	return (initial);
}

static int	request_and_wait(t_sim *sim, t_coder *coder)
{
	t_request	request;
	int			granted;
	int			initial;

	granted = 0;
	initial = prepare_request(sim, coder, &request, &granted);
	if (!heap_insert(&sim->pair_queue, request))
		return (0);
	if (initial)
		sim->initial_requests++;
	if (sim->initial_requests >= sim->config->number_of_coders)
	{
		if (try_grant_pairs(sim) > 0)
			pthread_cond_broadcast(&sim->pair_cond);
	}
	return (wait_for_grant(sim, coder, &granted));
}

int	scheduler_acquire_pair(t_coder *coder)
{
	t_sim	*sim;
	int		ok;

	sim = coder->sim;
	pthread_mutex_lock(&sim->pair_mutex);
	if (sim_is_stopped(sim))
		ok = 0;
	else
		ok = request_and_wait(sim, coder);
	pthread_mutex_unlock(&sim->pair_mutex);
	return (ok);
}

void	scheduler_release_pair(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->pair_mutex);
	if (try_grant_pairs(sim) > 0)
		pthread_cond_broadcast(&sim->pair_cond);
	pthread_mutex_unlock(&sim->pair_mutex);
}

void	scheduler_dispatch(t_sim *sim)
{
	pthread_mutex_lock(&sim->pair_mutex);
	if (try_grant_pairs(sim) > 0)
		pthread_cond_broadcast(&sim->pair_cond);
	pthread_mutex_unlock(&sim->pair_mutex);
}
