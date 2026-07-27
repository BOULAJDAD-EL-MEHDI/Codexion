/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 14:59:57 by eboulajd          #+#    #+#             */
/*   Updated: 2026/07/27 00:54:25 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H
 
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <unistd.h>
# include <sys/time.h>
 
typedef struct s_config
{
	int		number_of_coders;
	int		time_to_burnout;
	int		time_to_compile;
	int		time_to_debug;
	int		time_to_refactor;
	int		number_of_compiles_required;
	int		dongle_cooldown;
	char	*scheduler;
}	t_config;
 
typedef struct s_sim	t_sim;
 
typedef struct s_dongle
{
	int				id;
	pthread_mutex_t	mutex;
}	t_dongle;
 
typedef struct s_coder
{
	int				id;
	int				compiles_done;
	long			last_action_time;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	pthread_t		thread;
	pthread_mutex_t	action_mutex;
	t_sim			*sim;
}	t_coder;
 
struct s_sim
{
	t_config		*config;
	t_coder			*coders;
	t_dongle		*dongles;
	int				stop;
	int				coders_done;
	long			start_time;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	write_mutex;
};
 
int		parse_args(int argc, char **argv, t_config *config);
long	custom_atoi(char *str);
long	handle_positive(char *str, long *buffer);
long	handle_negative(char *str);
long	get_time_ms(void);
int		init_sim(t_sim *sim, t_config *config);
int		start_simulation(t_sim *sim);
void	destroy_sim(t_sim *sim);
void	*coder_routine(void *arg);
 
#endif
 