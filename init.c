/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 22:25:02 by eboulajd          #+#    #+#             */
/*   Updated: 2026/07/27 00:56:05 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

 
long	get_time_ms(void)
{
	struct timeval	tv;
 
	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}
 
static int	alloc_arrays(t_sim *sim, int count)
{
	sim->dongles = malloc(sizeof(t_dongle) * count);
	if (!sim->dongles)
		return (0);
	sim->coders = malloc(sizeof(t_coder) * count);
	if (!sim->coders)
	{
		free(sim->dongles);
		return (0);
	}
	return (1);
}
 
static int	init_dongles(t_dongle *dongles, int count)
{
	int	i;
 
	i = 0;
	while (i < count)
	{
		dongles[i].id = i;
		if (pthread_mutex_init(&dongles[i].mutex, NULL) != 0)
		{
			while (--i >= 0)
				pthread_mutex_destroy(&dongles[i].mutex);
			return (0);
		}
		i++;
	}
	return (1);
}
 
static int	init_coders(t_sim *sim, int count)
{
	int	i;
	int	j;
 
	i = 0;
	while (i < count)
	{
		sim->coders[i].id = i;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].last_action_time = sim->start_time;
		sim->coders[i].left_dongle = &sim->dongles[i];
		sim->coders[i].right_dongle = &sim->dongles[(i + 1) % count];
		sim->coders[i].sim = sim;
		if (pthread_mutex_init(&sim->coders[i].action_mutex, NULL) != 0)
		{
			j = 0;
			while (j < i)
				pthread_mutex_destroy(&sim->coders[j++].action_mutex);
			return (0);
		}
		i++;
	}
	return (1);
}
 
int	init_sim(t_sim *sim, t_config *config)
{
	int	count;
 
	count = config->number_of_coders;
	sim->config = config;
	sim->stop = 0;
	sim->coders_done = 0;
	sim->start_time = get_time_ms();
	if (pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->write_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->stop_mutex);
		return (0);
	}
	if (!alloc_arrays(sim, count))
		return (0);
	if (!init_dongles(sim->dongles, count) || !init_coders(sim, count))
	{
		free(sim->dongles);
		free(sim->coders);
		return (0);
	}
	return (1);
}