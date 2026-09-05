/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:19:58 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:19:59 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"
#include <unistd.h>

static int	find_burned_out_coder(t_sim *sim, long now)
{
	int	i;
	int	count;

	i = 0;
	count = sim->config->number_of_coders;
	while (i < count)
	{
		if (coder_burned_out(&sim->coders[i], now))
			return (i);
		i++;
	}
	return (-1);
}

static int	monitor_simulation(t_sim *sim)
{
	long	now;
	int		burned_id;

	scheduler_dispatch(sim);
	now = get_time_ms();
	burned_id = find_burned_out_coder(sim, now);
	if (burned_id != -1
		&& report_burnout(sim, &sim->coders[burned_id], now))
	{
		stop_simulation(sim);
		return (0);
	}
	if (check_completion(sim))
	{
		stop_simulation(sim);
		return (0);
	}
	return (1);
}

static void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (monitor_simulation(sim))
		usleep(200);
	return (NULL);
}

int	create_monitor_thread(t_sim *sim)
{
	if (pthread_create(&sim->monitor_thread, NULL,
			monitor_routine, sim) != 0)
		return (0);
	return (1);
}
