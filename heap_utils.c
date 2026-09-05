/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:16:33 by eboulajd          #+#    #+#             */
/*   Updated: 2026/09/05 12:19:15 by eboulajd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

static void	swap_requests(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static int	request_has_priority(t_request a, t_request b)
{
	if (a.priority != b.priority)
		return (a.priority < b.priority);
	if (a.sequence != b.sequence)
		return (a.sequence < b.sequence);
	return (a.coder_id < b.coder_id);
}

void	heapify_up(t_heap *heap, int index)
{
	int	parent;

	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!request_has_priority(heap->data[index], heap->data[parent]))
			break ;
		swap_requests(&heap->data[index], &heap->data[parent]);
		index = parent;
	}
}

void	heapify_down(t_heap *heap, int index)
{
	int	left;
	int	right;
	int	best;

	left = 2 * index + 1;
	right = 2 * index + 2;
	best = index;
	if (left < heap->size
		&& request_has_priority(heap->data[left], heap->data[best]))
		best = left;
	if (right < heap->size
		&& request_has_priority(heap->data[right], heap->data[best]))
		best = right;
	if (best != index)
	{
		swap_requests(&heap->data[index], &heap->data[best]);
		heapify_down(heap, best);
	}
}

int	heap_remove_coder(t_heap *heap, int coder_id, t_request *out)
{
	int	i;

	i = 0;
	while (i < heap->size && heap->data[i].coder_id != coder_id)
		i++;
	if (i == heap->size)
		return (0);
	*out = heap->data[i];
	heap->size--;
	if (i != heap->size)
	{
		heap->data[i] = heap->data[heap->size];
		heapify_down(heap, i);
		heapify_up(heap, i);
	}
	return (1);
}
