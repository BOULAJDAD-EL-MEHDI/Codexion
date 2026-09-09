/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:20:24 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:20:25 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	refresh_dongles(t_sim *sim, long now)
{
	int	i;

	i = 0;
	while (i < sim->config->number_of_coders)
	{
		if (!sim->dongles[i].available
			&& sim->dongles[i].available_at > 0
			&& sim->dongles[i].available_at <= now)
		{
			sim->dongles[i].available = 1;
			sim->dongles[i].available_at = 0;
		}
		i++;
	}
}

static int	request_is_alive(t_sim *sim, t_request *request, long now)
{
	t_coder	*coder;
	long	last_start;

	coder = &sim->coders[request->coder_id];
	pthread_mutex_lock(&coder->action_mutex);
	last_start = coder->last_compile_start;
	pthread_mutex_unlock(&coder->action_mutex);
	return (now - last_start < sim->config->time_to_burnout);
}

static int	grant_request(t_sim *sim, t_request *request, long now)
{
	t_dongle	*left;
	t_dongle	*right;

	left = sim->coders[request->coder_id].left_dongle;
	right = sim->coders[request->coder_id].right_dongle;
	if (left == right || !left->available || !right->available)
		return (0);
	if (!request_is_alive(sim, request, now))
		return (0);
	left->available = 0;
	right->available = 0;
	left->available_at = now + sim->config->time_to_compile
		+ left->cooldown_ms;
	right->available_at = left->available_at;
	pthread_mutex_lock(&sim->coders[request->coder_id].action_mutex);
	sim->coders[request->coder_id].last_compile_start = now;
	pthread_mutex_unlock(&sim->coders[request->coder_id].action_mutex);
	*request->granted = 1;
	return (1);
}

static void	restore_pending_requests(t_sim *sim)
{
	t_request	request;

	while (heap_remove_min(&sim->pair_scratch, &request))
		heap_insert(&sim->pair_queue, request);
	return ;
}

int	try_grant_pairs(t_sim *sim)
{
	t_request	request;
	long		now;
	int			grants;

	if (sim->initial_requests < sim->config->number_of_coders)
		return (0);
	now = get_time_ms();
	grants = 0;
	refresh_dongles(sim, now);
	while (heap_remove_min(&sim->pair_queue, &request))
	{
		if (grant_request(sim, &request, now))
			grants++;
		else
			heap_insert(&sim->pair_scratch, request);
	}
	restore_pending_requests(sim);
	return (grants);
}
