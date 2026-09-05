/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:19:08 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:19:09 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>

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

typedef struct s_request
{
	int		coder_id;
	long	priority;
	long	sequence;
	int		*granted;
}	t_request;

typedef struct s_heap
{
	t_request	*data;
	int			size;
	int			capacity;
}	t_heap;

typedef struct s_dongle
{
	int		available;
	int		cooldown_ms;
	long	available_at;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	int				compiles_done;
	long			last_compile_start;
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
	int				coders_started;
	int				monitor_started;
	int				start_ready;
	int				initial_requests;
	int				timer_waiting;
	long			start_time;
	long			pair_sequence;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	write_mutex;
	pthread_mutex_t	pair_mutex;
	pthread_cond_t	pair_cond;
	t_heap			pair_queue;
	t_heap			pair_scratch;
	pthread_t		monitor_thread;
};

int		parse_args(int argc, char **argv, t_config *config);
long	custom_atoi(char *str);
long	get_time_ms(void);
int		sim_is_stopped(t_sim *sim);
void	sim_sleep(t_sim *sim, int duration_ms);
int		init_sim(t_sim *sim, t_config *config);
int		init_sim_mutexes(t_sim *sim);
void	destroy_sim_mutexes(t_sim *sim);
int		init_pair_queues(t_sim *sim, int count);
int		prepare_arrays(t_sim *sim, int count);
int		start_simulation(t_sim *sim);
void	cleanup_sim(t_sim *sim);
void	*coder_routine(void *arg);
int		create_monitor_thread(t_sim *sim);
int		coder_burned_out(t_coder *coder, long now);
int		report_burnout(t_sim *sim, t_coder *coder, long now);
int		check_completion(t_sim *sim);
void	stop_simulation(t_sim *sim);
int		heap_init(t_heap *heap, int capacity);
void	heap_destroy(t_heap *heap);
int		heap_insert(t_heap *heap, t_request request);
int		heap_remove_min(t_heap *heap, t_request *out);
int		heap_remove_coder(t_heap *heap, int coder_id, t_request *out);
void	heapify_up(t_heap *heap, int index);
void	heapify_down(t_heap *heap, int index);
void	init_dongle_scheduling(t_dongle *dongles, int count, int cooldown_ms);
long	compute_pair_priority(t_sim *sim, t_coder *coder);
int		coder_public_id(t_coder *coder);
int		scheduler_acquire_pair(t_coder *coder);
void	scheduler_release_pair(t_coder *coder);
void	scheduler_dispatch(t_sim *sim);

#endif
