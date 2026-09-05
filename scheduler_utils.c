/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:20:18 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:20:19 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

int	coder_public_id(t_coder *coder)
{
	return (coder->id + 1);
}

long	compute_pair_priority(t_sim *sim, t_coder *coder)
{
	long	priority;

	if (strcmp(sim->config->scheduler, "fifo") == 0)
		return (sim->pair_sequence);
	pthread_mutex_lock(&coder->action_mutex);
	priority = coder->last_compile_start + sim->config->time_to_burnout;
	pthread_mutex_unlock(&coder->action_mutex);
	return (priority);
}

void	init_dongle_scheduling(t_dongle *dongles, int count, int cooldown_ms)
{
	int	i;

	i = 0;
	while (i < count)
	{
		dongles[i].available = 1;
		dongles[i].cooldown_ms = cooldown_ms;
		dongles[i].available_at = 0;
		i++;
	}
}
