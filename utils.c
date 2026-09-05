/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:20:38 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:20:39 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"
#include <unistd.h>

int	sim_is_stopped(t_sim *sim)
{
	int	stopped;

	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stopped);
}

void	sim_sleep(t_sim *sim, int duration_ms)
{
	long	target;

	if (duration_ms <= 0)
		return ;
	target = get_time_ms() + duration_ms;
	while (get_time_ms() < target && !sim_is_stopped(sim))
		usleep(1000);
}
