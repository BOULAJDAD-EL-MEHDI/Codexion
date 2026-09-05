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

static int	try_grant_pairs(t_sim *sim)
{
	t_request	request;
	long		now;
	int		grants;

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

static int	wait_for_grant(t_sim *sim, t_coder *coder, int *granted)
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
